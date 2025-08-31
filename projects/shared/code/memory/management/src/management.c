#include "shared/memory/management/management.h"

#include "abstraction/interrupts.h"
#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/converter.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/page.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/memory-manager.h"

MMTreeWithFreeList virtualMA;
MMTreeWithFreeList physicalMA;

void insertMMNodeAndAddToFreelist(MMTreeWithFreeList *treeWithFreeList,
                                  MMNode *newNode) {
    InsertResult insertResult = insertMMNode(treeWithFreeList, newNode);

    for (typeof_unqual(RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) i = 0;
         (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
         insertResult.freed[i];
         i++) {
        treeWithFreeList->freeList.buf[treeWithFreeList->freeList.len] =
            insertResult.freed[i];
        treeWithFreeList->freeList.len++;
    }
}

static void insertMemory(Memory memory, MMTreeWithFreeList *allocator) {
    U32 newNodeIndex =
        getNodeFromTreeWithFreeList((TreeWithFreeList *)allocator);

    if (!newNodeIndex) {
        interruptUnexpectedError();
    }

    MMNode *newNode = getMMNode(allocator, newNodeIndex);
    newNode->data.memory = memory;

    insertMMNodeAndAddToFreelist(allocator, newNode);
}

static U32 getMemoryAllocation(MMTreeWithFreeList *allocator, U64 bytes) {
    U32 availableMemory = deleteAtLeastMMNode(allocator, bytes);
    if (!availableMemory) {
        if (allocator == &physicalMA) {
            interruptNoMorePhysicalMemory();
        } else {
            interruptNoMoreVirtualMemory();
        }
    }
    return availableMemory;
}

void freeVirtualMemory(Memory memory) { insertMemory(memory, &virtualMA); }

void freePhysicalMemory(Memory memory) { insertMemory(memory, &physicalMA); }

static U64 alignedToTotal(U64 bytes, U64_pow2 align) {
    return bytes + align - 1;
}

static void handleRemovedAllocator(MMNode *availableMemory,
                                   U32 availableMemoryIndex, Memory memoryUsed,
                                   MMTreeWithFreeList *allocator) {
    U64 beforeResultBytes =
        memoryUsed.start - availableMemory->data.memory.start;
    U64 afterResultBytes = availableMemory->data.memory.bytes -
                           (beforeResultBytes + memoryUsed.bytes);

    if (beforeResultBytes && afterResultBytes) {
        availableMemory->data.memory.bytes = beforeResultBytes;
        (void)insertMMNode(allocator, availableMemory);

        insertMemory((Memory){.start = memoryUsed.start + memoryUsed.bytes,
                              .bytes = afterResultBytes},
                     allocator);
    } else if (beforeResultBytes) {
        availableMemory->data.memory.bytes = beforeResultBytes;
        (void)insertMMNode(allocator, availableMemory);
    } else if (afterResultBytes) {
        availableMemory->data.memory =
            (Memory){.start = memoryUsed.start + memoryUsed.bytes,
                     .bytes = afterResultBytes};
        (void)insertMMNode(allocator, availableMemory);
    } else {
        allocator->freeList.buf[allocator->freeList.len] = availableMemoryIndex;
        allocator->freeList.len++;
    }
}

static void *allocAlignedMemory(U64 bytes, U64_pow2 align,
                                MMTreeWithFreeList *allocator) {
    U32 availableMemoryIndex =
        getMemoryAllocation(allocator, alignedToTotal(bytes, align));
    MMNode *availableMemoryNode = getMMNode(allocator, availableMemoryIndex);
    U64 result = alignUp(availableMemoryNode->data.memory.start, align);
    handleRemovedAllocator(availableMemoryNode, availableMemoryIndex,
                           (Memory){.start = result, .bytes = bytes},
                           allocator);
    return (void *)result;
}

void *allocVirtualMemory(U64 bytes, U64_pow2 align) {
    return allocAlignedMemory(bytes, align, &virtualMA);
}

void *allocPhysicalMemory(U64 bytes, U64_pow2 align) {
    return allocAlignedMemory(bytes, align, &physicalMA);
}

static void initMemoryAllocator(PackedTreeWithFreeList *packedMemoryAllocator,
                                TreeWithFreeList *allocator) {
    allocator->buf = packedMemoryAllocator->buf;
    allocator->len = packedMemoryAllocator->len;
    allocator->cap = packedMemoryAllocator->cap;
    allocator->elementSizeBytes = packedMemoryAllocator->elementSizeBytes;

    allocator->freeList.buf = packedMemoryAllocator->freeList.buf;
    allocator->freeList.cap = packedMemoryAllocator->freeList.cap;
    allocator->freeList.len = packedMemoryAllocator->freeList.len;

    allocator->rootIndex = packedMemoryAllocator->rootIndex;
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;

static void arrayLikeMappable(void **buf, U32 *cap, U32 len,
                              U64_pow2 alignBytes, U64 elementSizeBytes,
                              U32 additionalMaps) {
    void *virtualBuffer =
        allocVirtualMemory(ALLOCATOR_MAX_BUFFER_SIZE, alignBytes);

    U64 bytesUsed = len * elementSizeBytes;
    U32 mapsToDo =
        (U32)ceilingDivide(bytesUsed, pageSizesSmallest()) + additionalMaps;
    for (typeof(mapsToDo) i = 0; i < mapsToDo; i++) {
        (void)handlePageFault((U64)virtualBuffer + (i * pageSizesSmallest()));
    }

    memcpy(virtualBuffer, *buf, len * elementSizeBytes);
    *buf = virtualBuffer;
    *cap = ALLOCATOR_MAX_BUFFER_SIZE / elementSizeBytes;
}

static void IdentityToMappable(TreeWithFreeList *treeWithFreeList,
                               U64_pow2 alignBytes, U64 elementSizeBytes,
                               U32 additionalMaps) {
    arrayLikeMappable(&treeWithFreeList->buf, &treeWithFreeList->cap,
                      treeWithFreeList->len, alignBytes, elementSizeBytes,
                      additionalMaps);

    arrayLikeMappable((void **)&treeWithFreeList->freeList.buf,
                      &treeWithFreeList->freeList.cap,
                      treeWithFreeList->freeList.len, alignof(U32), sizeof(U32),
                      0);
}

static void freePackedVMMTree(PackedVMMTreeWithFreeList *packed) {
    freePhysicalMemory((Memory){.start = (U64)packed->buf,
                                .bytes = packed->cap * sizeof(*packed->buf)});
    freePhysicalMemory((Memory){.start = (U64)packed->freeList.buf,
                                .bytes = packed->freeList.cap *
                                         sizeof(*packed->freeList.buf)});
}

static void freePackedMMTree(PackedMMTreeWithFreeList *packed) {
    freePhysicalMemory((Memory){.start = (U64)packed->buf,
                                .bytes = packed->cap * sizeof(*packed->buf)});
    freePhysicalMemory((Memory){.start = (U64)packed->freeList.buf,
                                .bytes = packed->freeList.cap *
                                         sizeof(*packed->freeList.buf)});
}

void initMemoryManagers(PackedKernelMemory *kernelMemory) {
    initMemoryAllocator((PackedTreeWithFreeList *)&kernelMemory->physicalPMA,
                        (TreeWithFreeList *)&physicalMA);
    initMemoryAllocator((PackedTreeWithFreeList *)&kernelMemory->virtualPMA,
                        (TreeWithFreeList *)&virtualMA);
    initMemoryAllocator(
        (PackedTreeWithFreeList *)&kernelMemory->virtualMemorySizeMapper,
        (TreeWithFreeList *)&virtualMemorySizeMapper);

    // NOTE: Adding one extra map here because we are doing page faults manually
    // which will increase the physical memory usage
    IdentityToMappable((TreeWithFreeList *)&physicalMA,
                       alignof(*physicalMA.buf), sizeof(*physicalMA.buf), 1);
    IdentityToMappable((TreeWithFreeList *)&virtualMA, alignof(*virtualMA.buf),
                       sizeof(*virtualMA.buf), 0);
    IdentityToMappable((TreeWithFreeList *)&virtualMemorySizeMapper,
                       alignof(*virtualMemorySizeMapper.buf),
                       sizeof(*virtualMemorySizeMapper.buf), 0);

    freePackedMMTree(&kernelMemory->physicalPMA);
    freePackedMMTree(&kernelMemory->virtualPMA);
    freePackedVMMTree(&kernelMemory->virtualMemorySizeMapper);
}
