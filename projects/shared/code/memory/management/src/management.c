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

void insertMMNodeAndAddToFreelist(RedBlackMMTreeWithFreeList *allocator,
                                  MMNode *newNode) {
    InsertResult insertResult = insertMMNode(&allocator->tree, newNode);

    for (typeof_unqual(RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) i = 0;
         (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
         insertResult.freed[i];
         i++) {
        nodeAllocatorFree(&allocator->nodeAllocator, insertResult.freed[i]);
    }
}

static void insertMemory(Memory memory, RedBlackMMTreeWithFreeList *allocator) {
    MMNode *newNode = nodeAllocatorGet(&allocator->nodeAllocator);
    if (!newNode) {
        interruptUnexpectedError();
    }
    newNode->memory = memory;
    insertMMNodeAndAddToFreelist(allocator, newNode);
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

static U64 alignedToTotal(U64 bytes, U64_pow2 align) {
    return bytes + align - 1;
}

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
        nodeAllocatorFree(&allocator->nodeAllocator, availableMemory);
    }
}

static void *allocAlignedMemory(U64 bytes, U64_pow2 align,
                                RedBlackMMTreeWithFreeList *allocator) {
    MMNode *availableMemory =
        getMemoryAllocation(allocator, alignedToTotal(bytes, align));
    U64 result = alignUp(availableMemory->memory.start, align);
    handleRemovedAllocator(
        availableMemory, (Memory){.start = result, .bytes = bytes}, allocator);
    return (void *)result;
}

void *allocVirtualMemory(U64 bytes, U64_pow2 align) {
    return allocAlignedMemory(bytes, align, &virtualMA);
}

void *allocPhysicalMemory(U64 bytes, U64_pow2 align) {
    return allocAlignedMemory(bytes, align, &physicalMA);
}

static void initMemoryAllocator(NodeAllocator *nodeAllocator, void **tree,
                                NodeAllocator *nodeAllocatorParam,
                                void *packedTree) {
    nodeAllocator->nodes.buf = nodeAllocatorParam->nodes.buf;
    nodeAllocator->nodes.len = nodeAllocatorParam->nodes.len;
    nodeAllocator->nodes.cap = nodeAllocatorParam->nodes.cap;

    nodeAllocator->nodesFreeList.buf = nodeAllocatorParam->nodesFreeList.buf;
    nodeAllocator->nodesFreeList.cap = nodeAllocatorParam->nodesFreeList.cap;
    nodeAllocator->nodesFreeList.len = nodeAllocatorParam->nodesFreeList.len;

    nodeAllocator->elementSizeBytes = nodeAllocatorParam->elementSizeBytes;
    nodeAllocator->alignBytes = nodeAllocatorParam->alignBytes;

    *tree = packedTree;
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;

static void identityArrayToMappable(void_max_a *array, U32_pow2 alignBytes,
                                    U32 elementSizeBytes, U32 additionalMaps) {
    void *virtualBuffer =
        allocVirtualMemory(ALLOCATOR_MAX_BUFFER_SIZE, alignBytes);

    U64 bytesUsed = array->len * elementSizeBytes;
    U32 mapsToDo =
        (U32)ceilingDivide(bytesUsed, pageSizesSmallest()) + additionalMaps;
    for (typeof(mapsToDo) i = 0; i < mapsToDo; i++) {
        (void)handlePageFault((U64)virtualBuffer + (i * pageSizesSmallest()));
    }

    memcpy(virtualBuffer, array->buf, array->len * elementSizeBytes);
    array->buf = virtualBuffer;
    array->cap = ALLOCATOR_MAX_BUFFER_SIZE / elementSizeBytes;
}

typedef struct {
    RedBlackNodeBasic_max_a nodes;
    RedBlackNodeBasic *tree;
    RedBlackNodeBasicPtr_max_a freeList;
} RedBlackNodeBasicTreeWithFreeList;

static void treeWithFreeListToMappable(NodeAllocator *nodeAllocator,
                                       void **tree,
                                       U32 additionalMapsForNodeBuffer) {
    U64 originalBufferLocation = (U64)nodeAllocator->nodes.buf;

    identityArrayToMappable(
        (void_max_a *)&nodeAllocator->nodes, nodeAllocator->alignBytes,
        nodeAllocator->elementSizeBytes, additionalMapsForNodeBuffer);
    U64 newNodesLocation = (U64)nodeAllocator->nodes.buf;
    U64 nodesBias = newNodesLocation - originalBufferLocation;
    for (typeof(nodeAllocator->nodes.len) i = 0; i < nodeAllocator->nodes.len;
         i++) {
        U8 *base = (U8 *)nodeAllocator->nodes.buf;
        RedBlackNodeBasic **children =
            (RedBlackNodeBasic **)(base +
                                   (i * nodeAllocator->elementSizeBytes));

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

    if (*tree) {
        *tree = (RedBlackNodeBasic *)((U8 *)(*tree) + nodesBias);
    }

    for (typeof(nodeAllocator->nodesFreeList.len) i = 0;
         i < nodeAllocator->nodesFreeList.len; i++) {
        nodeAllocator->nodesFreeList.buf[i] =
            (RedBlackNodeBasic *)((U8 *)nodeAllocator->nodesFreeList.buf[i] +
                                  nodesBias);
    }

    identityArrayToMappable((void_max_a *)&nodeAllocator->nodesFreeList,
                            alignof(*nodeAllocator->nodesFreeList.buf),
                            sizeof(*nodeAllocator->nodesFreeList.buf), 0);
}

static void freePackedNodeAllocator(NodeAllocator *nodeAllocatorParam) {
    freePhysicalMemory((Memory){.start = (U64)nodeAllocatorParam->nodes.buf,
                                .bytes = nodeAllocatorParam->nodes.cap *
                                         nodeAllocatorParam->elementSizeBytes});
    freePhysicalMemory(
        (Memory){.start = (U64)nodeAllocatorParam->nodesFreeList.buf,
                 .bytes = nodeAllocatorParam->nodesFreeList.cap *
                          sizeof(*nodeAllocatorParam->nodesFreeList.buf)});
}

void initMemoryManagers(KernelMemory *kernelMemory) {
    initMemoryAllocator(&physicalMA.nodeAllocator, (void **)&physicalMA.tree,
                        &kernelMemory->physicalPMA.nodeAllocator,
                        kernelMemory->physicalPMA.tree);
    initMemoryAllocator(&virtualMA.nodeAllocator, (void **)&virtualMA.tree,
                        &kernelMemory->virtualPMA.nodeAllocator,
                        kernelMemory->virtualPMA.tree);
    initMemoryAllocator(&virtualMemorySizeMapper.nodeAllocator,
                        (void **)&virtualMemorySizeMapper.tree,
                        &kernelMemory->virtualMemorySizeMapper.nodeAllocator,
                        kernelMemory->virtualMemorySizeMapper.tree);

    // NOTE: Adding one extra map here because we are doing page faults manually
    // which will increase the physical memory usage
    treeWithFreeListToMappable(&physicalMA.nodeAllocator,
                               (void **)&physicalMA.tree, 1);
    treeWithFreeListToMappable(&virtualMA.nodeAllocator,
                               (void **)&virtualMA.tree, 0);

    treeWithFreeListToMappable(&virtualMemorySizeMapper.nodeAllocator,
                               (void **)&virtualMemorySizeMapper.tree, 0);

    freePackedNodeAllocator(&kernelMemory->physicalPMA.nodeAllocator);
    freePackedNodeAllocator(&kernelMemory->virtualPMA.nodeAllocator);
    freePackedNodeAllocator(
        &kernelMemory->virtualMemorySizeMapper.nodeAllocator);
}
