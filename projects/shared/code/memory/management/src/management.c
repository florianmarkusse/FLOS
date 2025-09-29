#include "shared/memory/management/management.h"

#include "abstraction/interrupts.h"
#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/converter.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/page.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/memory-manager.h"

BuddyWithNodeAllocator buddyPhysical;
BuddyWithNodeAllocator buddyVirtual;

void freeVirtualMemory(Memory memory) {
    buddyFree(&buddyVirtual.buddy, memory, &buddyVirtual.nodeAllocator);
}

void freePhysicalMemory(Memory memory) {
    buddyFree(&buddyPhysical.buddy, memory, &buddyPhysical.nodeAllocator);
}

void *allocVirtualMemory(U64_pow2 blockSize) {
    return buddyAllocate(&buddyVirtual.buddy, blockSize,
                         &buddyVirtual.nodeAllocator);
}

void *allocPhysicalMemory(U64_pow2 blockSize) {
    return buddyAllocate(&buddyPhysical.buddy, blockSize,
                         &buddyPhysical.nodeAllocator);
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;

static void identityArrayToMappable(void_max_a *array, U32 elementSizeBytes,
                                    U32 additionalMaps) {
    void *virtualBuffer = allocVirtualMemory(ALLOCATOR_MAX_BUFFER_SIZE);

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

static void treeWithFreeListToMappable(NodeAllocator *nodeAllocator,
                                       void **trees, U32 treesLen,
                                       U32 additionalMapsForNodeBuffer) {
    U64 originalBufferLocation = (U64)nodeAllocator->nodes.buf;

    identityArrayToMappable((void_max_a *)&nodeAllocator->nodes,
                            nodeAllocator->elementSizeBytes,
                            additionalMapsForNodeBuffer);
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

    for (typeof(treesLen) i = 0; i < treesLen; i++) {
        if (trees[i]) {
            trees[i] = (((U8 *)trees[i]) + nodesBias);
        }
    }

    for (typeof(nodeAllocator->nodesFreeList.len) i = 0;
         i < nodeAllocator->nodesFreeList.len; i++) {
        nodeAllocator->nodesFreeList.buf[i] =
            (RedBlackNodeBasic *)((U8 *)nodeAllocator->nodesFreeList.buf[i] +
                                  nodesBias);
    }

    identityArrayToMappable((void_max_a *)&nodeAllocator->nodesFreeList,
                            sizeof(*nodeAllocator->nodesFreeList.buf), 0);
}

void initMemoryManagers(KernelMemory *kernelMemory) {
    buddyPhysical.nodeAllocator = kernelMemory->buddyPhysical.nodeAllocator;
    buddyPhysical.buddy.data = kernelMemory->buddyPhysical.data;
    if (setjmp(buddyPhysical.buddy.jmpBuf)) {
        interruptNoMorePhysicalMemory();
    }

    buddyVirtual.nodeAllocator = kernelMemory->buddyVirtual.nodeAllocator;
    buddyVirtual.buddy.data = kernelMemory->buddyVirtual.data;
    if (setjmp(buddyVirtual.buddy.jmpBuf)) {
        interruptNoMoreVirtualMemory();
    }

    memoryMapperSizes = kernelMemory->memoryMapperSizes;

    // NOTE: Adding one extra map here because we are doing page faults manually
    // which will increase the physical memory usage
    treeWithFreeListToMappable(&buddyPhysical.nodeAllocator,
                               (void **)&buddyPhysical.buddy.data.blocksFree,
                               buddyOrderCount(&buddyPhysical.buddy), 1);
    treeWithFreeListToMappable(&buddyVirtual.nodeAllocator,
                               (void **)&buddyVirtual.buddy.data.blocksFree,
                               buddyOrderCount(&buddyVirtual.buddy), 0);
    treeWithFreeListToMappable(&memoryMapperSizes.nodeAllocator,
                               (void **)&memoryMapperSizes.tree, 1, 0);
}
