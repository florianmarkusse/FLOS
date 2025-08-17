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

RedBlackMMTreeWithFreeList virtualMA;
RedBlackMMTreeWithFreeList physicalMA;

void insertMMNodeAndAddToFreelist(MMNode **root, MMNode *newNode,
                                  MMNodePtr_max_a *freeList) {
    InsertResult insertResult = insertMMNode(root, newNode);

    for (U32 i = 0; (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
                    insertResult.freed[i];
         i++) {
        freeList->buf[freeList->len] = insertResult.freed[i];
        freeList->len++;
    }
}

// TODO: Ready for code generation
MMNode *getMMNode(MMNodePtr_max_a *freeList, MMNode_max_a *nodes) {
    if (freeList->len > 0) {
        MMNode *result = freeList->buf[freeList->len - 1];
        freeList->len--;
        return result;
    }

    if (nodes->len < nodes->cap) {
        MMNode *result = &nodes->buf[nodes->len];
        nodes->len++;
        return result;
    }

    return nullptr;
}

static void insertMemory(Memory memory, RedBlackMMTreeWithFreeList *allocator) {
    MMNode *newNode = getMMNode(&allocator->freeList, &allocator->nodes);
    if (!newNode) {
        interruptUnexpectedError();
    }
    newNode->memory = memory;

    insertMMNodeAndAddToFreelist(&allocator->tree, newNode,
                                 &allocator->freeList);
}

static MMNode *getMemoryAllocation(RedBlackMMTreeWithFreeList *allocator,
                                   U64 bytes) {
    MMNode *availableMemory = deleteAtLeastMMNode(&allocator->tree, bytes);
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

static void handleRemovedAllocator(MMNode *availableMemory, Memory memoryUsed,
                                   RedBlackMMTreeWithFreeList *allocator) {
    U64 beforeResultBytes = memoryUsed.start - availableMemory->memory.start;
    U64 afterResultBytes =
        availableMemory->memory.bytes - (beforeResultBytes + memoryUsed.bytes);

    if (beforeResultBytes && afterResultBytes) {
        availableMemory->memory.bytes = beforeResultBytes;
        (void)insertMMNode(&allocator->tree, availableMemory);

        insertMemory((Memory){.start = memoryUsed.start + memoryUsed.bytes,
                              .bytes = afterResultBytes},
                     allocator);
    } else if (beforeResultBytes) {
        availableMemory->memory.bytes = beforeResultBytes;
        (void)insertMMNode(&allocator->tree, availableMemory);
    } else if (afterResultBytes) {
        availableMemory->memory =
            (Memory){.start = memoryUsed.start + memoryUsed.bytes,
                     .bytes = afterResultBytes};
        (void)insertMMNode(&allocator->tree, availableMemory);
    } else {
        allocator->freeList.buf[allocator->freeList.len] = availableMemory;
        allocator->freeList.len++;
    }
}

static void *allocAlignedMemory(U64 bytes, U64 align,
                                RedBlackMMTreeWithFreeList *allocator) {
    MMNode *availableMemory =
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

static void initMemoryAllocator(PackedTreeWithFreeList *packedMemoryAllocator,
                                TreeWithFreeList *allocator) {
    allocator->nodes.buf = packedMemoryAllocator->nodes.buf;
    allocator->nodes.len = packedMemoryAllocator->nodes.len;
    allocator->nodes.cap = packedMemoryAllocator->nodes.cap;

    allocator->freeList.buf = packedMemoryAllocator->freeList.buf;
    allocator->freeList.cap = packedMemoryAllocator->freeList.cap;
    allocator->freeList.len = packedMemoryAllocator->freeList.len;

    allocator->tree = packedMemoryAllocator->tree;
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;

static void identityArrayToMappable(void_max_a *array, U64 alignBytes,
                                    U64 elementSizeBytes, U32 additionalMaps) {
    void *virtualBuffer =
        allocVirtualMemory(ALLOCATOR_MAX_BUFFER_SIZE, alignBytes);

    U64 bytesUsed = array->len * elementSizeBytes;
    U32 mapsToDo =
        (U32)CEILING_DIV_VALUE(bytesUsed, pageSizesSmallest()) + additionalMaps;
    for (typeof(mapsToDo) i = 0; i < mapsToDo; i++) {
        (void)handlePageFault((U64)virtualBuffer + (i * pageSizesSmallest()));
    }

    memcpy(virtualBuffer, array->buf, array->len * elementSizeBytes);
    array->buf = virtualBuffer;
    array->cap = ALLOCATOR_MAX_BUFFER_SIZE / elementSizeBytes;
}

// NOTE: Ready for code generation
static void VMMTreeToMappable(VMMTreeWithFreeList *memoryAllocator) {
    U64 originalBufferLocation = (U64)memoryAllocator->nodes.buf;

    identityArrayToMappable((void_max_a *)&memoryAllocator->nodes,
                            alignof(*memoryAllocator->nodes.buf),
                            sizeof(*memoryAllocator->nodes.buf), 0);
    U64 newNodesLocation = (U64)memoryAllocator->nodes.buf;
    U64 nodesBias = newNodesLocation - originalBufferLocation;
    for (typeof(memoryAllocator->nodes.len) i = 0;
         i < memoryAllocator->nodes.len; i++) {
        RedBlackNodeBasic **children =
            memoryAllocator->nodes.buf[i].basic.children;

        if (children[RB_TREE_LEFT]) {
            children[RB_TREE_LEFT] =
                (RedBlackNodeBasic *)(((U8 *)children[RB_TREE_LEFT]) +
                                      nodesBias);
        }
        if (children[RB_TREE_RIGHT]) {
            children[RB_TREE_RIGHT] =
                (RedBlackNodeBasic *)(((U8 *)children[RB_TREE_RIGHT]) +
                                      nodesBias);
        }
    }

    if (memoryAllocator->tree) {
        memoryAllocator->tree =
            (VMMNode *)((U8 *)memoryAllocator->tree + nodesBias);
    }

    for (typeof(memoryAllocator->freeList.len) i = 0;
         i < memoryAllocator->freeList.len; i++) {
        memoryAllocator->freeList.buf[i] =
            (VMMNode *)((U8 *)memoryAllocator->freeList.buf[i] + nodesBias);
    }

    identityArrayToMappable((void_max_a *)&memoryAllocator->freeList,
                            alignof(*memoryAllocator->freeList.buf),
                            sizeof(*memoryAllocator->freeList.buf), 0);
}

// NOTE: Ready for code generation
static void MMTreeToMappable(RedBlackMMTreeWithFreeList *memoryAllocator,
                             U32 additionalMapsForNodesBuffer) {
    U64 originalBufferLocation = (U64)memoryAllocator->nodes.buf;

    identityArrayToMappable((void_max_a *)&memoryAllocator->nodes,
                            alignof(*memoryAllocator->nodes.buf),
                            sizeof(*memoryAllocator->nodes.buf),
                            additionalMapsForNodesBuffer);
    U64 newNodesLocation = (U64)memoryAllocator->nodes.buf;
    U64 nodesBias = newNodesLocation - originalBufferLocation;
    for (typeof(memoryAllocator->nodes.len) i = 0;
         i < memoryAllocator->nodes.len; i++) {
        MMNode **children = memoryAllocator->nodes.buf[i].children;

        if (children[RB_TREE_LEFT]) {
            children[RB_TREE_LEFT] =
                (MMNode *)(((U8 *)children[RB_TREE_LEFT]) + nodesBias);
        }
        if (children[RB_TREE_RIGHT]) {
            children[RB_TREE_RIGHT] =
                (MMNode *)(((U8 *)children[RB_TREE_RIGHT]) + nodesBias);
        }
    }

    if (memoryAllocator->tree) {
        memoryAllocator->tree =
            (MMNode *)((U8 *)memoryAllocator->tree + nodesBias);
    }

    for (typeof(memoryAllocator->freeList.len) i = 0;
         i < memoryAllocator->freeList.len; i++) {
        memoryAllocator->freeList.buf[i] =
            (MMNode *)((U8 *)memoryAllocator->freeList.buf[i] + nodesBias);
    }

    identityArrayToMappable((void_max_a *)&memoryAllocator->freeList,
                            alignof(*memoryAllocator->freeList.buf),
                            sizeof(*memoryAllocator->freeList.buf), 0);
}

static void freePackedVMMTree(PackedVMMTreeWithFreeList *packed) {
    freePhysicalMemory(
        (Memory){.start = (U64)packed->nodes.buf,
                 .bytes = packed->nodes.cap * sizeof(*packed->nodes.buf)});
    freePhysicalMemory((Memory){.start = (U64)packed->freeList.buf,
                                .bytes = packed->freeList.cap *
                                         sizeof(*packed->freeList.buf)});
}

static void freePackedMMTree(PackedMMTreeWithFreeList *packed) {
    freePhysicalMemory(
        (Memory){.start = (U64)packed->nodes.buf,
                 .bytes = packed->nodes.cap * sizeof(*packed->nodes.buf)});
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
    MMTreeToMappable(&physicalMA, 1);
    MMTreeToMappable(&virtualMA, 0);
    VMMTreeToMappable(&virtualMemorySizeMapper);

    freePackedMMTree(&kernelMemory->physicalPMA);
    freePackedMMTree(&kernelMemory->virtualPMA);
    freePackedVMMTree(&kernelMemory->virtualMemorySizeMapper);
}
