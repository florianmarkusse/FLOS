#include "shared/memory/management/management.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/converter.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/page.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/memory-manager.h"

MemoryAllocator virtualMA;
MemoryAllocator physicalMA;

void insertRedBlackNodeMMAndAddToFreelist(RedBlackNodeMM **root,
                                          RedBlackNodeMM *newNode,
                                          RedBlackNodeMMPtr_max_a *freeList) {
    InsertResult insertResult = insertRedBlackNodeMM(root, newNode);

    for (U64 i = 0; (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
                    insertResult.freed[i];
         i++) {
        freeList->buf[freeList->len] = insertResult.freed[i];
        freeList->len++;
    }
}

RedBlackNodeMM *getRedBlackNodeMM(RedBlackNodeMMPtr_max_a *freeList,
                                  RedBlackNodeMM_max_a *nodes) {
    if (freeList->len > 0) {
        RedBlackNodeMM *result = freeList->buf[freeList->len - 1];
        freeList->len--;
        return result;
    }

    if (nodes->len < nodes->cap) {
        RedBlackNodeMM *result = &nodes->buf[nodes->len];
        nodes->len++;
        return result;
    }

    return nullptr;
}

static void insertMemory(Memory memory, MemoryAllocator *allocator) {
    RedBlackNodeMM *newNode =
        getRedBlackNodeMM(&allocator->freeList, &allocator->nodes);
    if (!newNode) {
        interruptUnexpectedError();
    }
    newNode->memory = memory;

    insertRedBlackNodeMMAndAddToFreelist(&allocator->tree, newNode,
                                         &allocator->freeList);
}

static RedBlackNodeMM *getMemoryAllocation(MemoryAllocator *allocator,
                                           U64 bytes) {
    RedBlackNodeMM *availableMemory =
        deleteAtLeastRedBlackNodeMM(&allocator->tree, bytes);
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

static U64 alignedToTotal(U64 bytes, U64 align) { return bytes + align - 1; }

static void handleRemovedAllocator(RedBlackNodeMM *availableMemory,
                                   Memory memoryUsed,
                                   MemoryAllocator *allocator) {
    U64 beforeResultBytes = memoryUsed.start - availableMemory->memory.start;
    U64 afterResultBytes =
        availableMemory->memory.bytes - (beforeResultBytes + memoryUsed.bytes);

    if (beforeResultBytes && afterResultBytes) {
        availableMemory->memory.bytes = beforeResultBytes;
        (void)insertRedBlackNodeMM(&allocator->tree, availableMemory);

        insertMemory((Memory){.start = memoryUsed.start + memoryUsed.bytes,
                              .bytes = afterResultBytes},
                     allocator);
    } else if (beforeResultBytes) {
        availableMemory->memory.bytes = beforeResultBytes;
        (void)insertRedBlackNodeMM(&allocator->tree, availableMemory);
    } else if (afterResultBytes) {
        availableMemory->memory =
            (Memory){.start = memoryUsed.start + memoryUsed.bytes,
                     .bytes = afterResultBytes};
        (void)insertRedBlackNodeMM(&allocator->tree, availableMemory);
    } else {
        allocator->freeList.buf[allocator->freeList.len] = availableMemory;
        allocator->freeList.len++;
    }
}

static void *allocAlignedMemory(U64 bytes, U64 align,
                                MemoryAllocator *allocator) {
    RedBlackNodeMM *availableMemory =
        getMemoryAllocation(allocator, alignedToTotal(bytes, align));
    U64 result = ALIGN_UP_VALUE(availableMemory->memory.start, align);
    handleRemovedAllocator(
        availableMemory, (Memory){.start = result, .bytes = bytes}, allocator);
    return (void *)result;
}

void *allocVirtualMemory(U64 bytes, U64 align) {
    return allocAlignedMemory(bytes, align, &virtualMA);
}

void *allocPhysicalMemory(U64 bytes, U64 align) {
    return allocAlignedMemory(bytes, align, &physicalMA);
}

static void initMemoryAllocator(PackedMemoryAllocator *packedMemoryAllocator,
                                MemoryAllocator *allocator) {
    allocator->nodes.buf = packedMemoryAllocator->nodes.buf;
    allocator->nodes.len = packedMemoryAllocator->nodes.len;
    allocator->nodes.cap = packedMemoryAllocator->nodes.cap;

    allocator->freeList.buf = packedMemoryAllocator->freeList.buf;
    allocator->freeList.cap = packedMemoryAllocator->freeList.cap;
    allocator->freeList.len = packedMemoryAllocator->freeList.len;

    allocator->tree = packedMemoryAllocator->tree;
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;
static constexpr auto ALLOCATOR_PAGE_SIZE = SMALLEST_VIRTUAL_PAGE;

static void identityArrayToMappable(void_ptr_max_a *array, U64 alignBytes,
                                    U64 elementSizeBytes, U64 additionalMaps) {
    void *virtualBuffer =
        allocVirtualMemory(ALLOCATOR_MAX_BUFFER_SIZE, alignBytes);

    U64 bytesUsed = array->len * elementSizeBytes;
    U64 mapsToDo =
        CEILING_DIV_VALUE(bytesUsed, (U64)ALLOCATOR_PAGE_SIZE) + additionalMaps;
    for (U64 i = 0; i < mapsToDo; i++) {
        handlePageFault((U64)virtualBuffer + (i * ALLOCATOR_PAGE_SIZE));
    }

    memcpy(virtualBuffer, array->buf, array->len * elementSizeBytes);
    array->buf = virtualBuffer;
    array->cap = ALLOCATOR_MAX_BUFFER_SIZE / elementSizeBytes;
}

static void identityAllocatorToMappable(MemoryAllocator *memoryAllocator,
                                        U64 additionalMapsForNodesBuffer) {
    U64 originalBufferLocation = (U64)memoryAllocator->nodes.buf;

    identityArrayToMappable((void_ptr_max_a *)&memoryAllocator->nodes,
                            alignof(*memoryAllocator->nodes.buf),
                            sizeof(*memoryAllocator->nodes.buf),
                            additionalMapsForNodesBuffer);
    U64 newNodesLocation = (U64)memoryAllocator->nodes.buf;
    U64 nodesBias = newNodesLocation - originalBufferLocation;
    for (U64 i = 0; i < memoryAllocator->nodes.len; i++) {
        RedBlackNodeMM **children = memoryAllocator->nodes.buf[i].children;

        if (children[RB_TREE_LEFT]) {
            children[RB_TREE_LEFT] =
                (RedBlackNodeMM *)(((U8 *)children[RB_TREE_LEFT]) + nodesBias);
        }
        if (children[RB_TREE_RIGHT]) {
            children[RB_TREE_RIGHT] =
                (RedBlackNodeMM *)(((U8 *)children[RB_TREE_RIGHT]) + nodesBias);
        }
    }

    identityArrayToMappable((void_ptr_max_a *)&memoryAllocator->freeList,
                            alignof(*memoryAllocator->freeList.buf),
                            sizeof(*memoryAllocator->freeList.buf), 0);
    for (U64 i = 0; i < memoryAllocator->freeList.len; i++) {
        memoryAllocator->freeList.buf[i] =
            (RedBlackNodeMM *)((U8 *)memoryAllocator->freeList.buf[i] +
                               nodesBias);
    }

    memoryAllocator->tree =
        (RedBlackNodeMM *)((U8 *)memoryAllocator->tree + nodesBias);
}

static void
freePackedMemoryAllocator(PackedMemoryAllocator *packedMemoryAllocator) {
    freePhysicalMemory(
        (Memory){.start = (U64)packedMemoryAllocator->nodes.buf,
                 .bytes = packedMemoryAllocator->nodes.cap *
                          sizeof(*packedMemoryAllocator->nodes.buf)});
    freePhysicalMemory(
        (Memory){.start = (U64)packedMemoryAllocator->freeList.buf,
                 .bytes = packedMemoryAllocator->freeList.cap *
                          sizeof(*packedMemoryAllocator->freeList.buf)});
}

void initMemoryManagers(PackedKernelMemory *kernelMemory) {
    initMemoryAllocator(&kernelMemory->physicalPMA, &physicalMA);
    initMemoryAllocator(&kernelMemory->virtualPMA, &virtualMA);

    // NOTE: Adding one extra map here because we are doing page faults manually
    // which will increase the physical memory usage
    identityAllocatorToMappable(&physicalMA, 1);
    identityAllocatorToMappable(&virtualMA, 0);

    freePackedMemoryAllocator(&kernelMemory->physicalPMA);
    freePackedMemoryAllocator(&kernelMemory->virtualPMA);
}

void initVirtualMemoryManager(PackedMemoryAllocator *virtualMemoryTree) {
    // TODO: fix this, we are running out of nodes to use for the memory tree,
    // not out of memory. Use virtual memory setup and then this is a fine
    // interrupt I think.
    initMemoryAllocator(virtualMemoryTree, &virtualMA);
}

void initPhysicalMemoryManager(PackedMemoryAllocator *physicalMemoryTree) {
    initMemoryAllocator(physicalMemoryTree, &physicalMA);
}
