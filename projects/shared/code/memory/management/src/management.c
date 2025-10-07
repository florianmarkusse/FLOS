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

Buddy buddyPhysical;
Buddy buddyVirtual;

void freeVirtualMemory(Memory memory) { buddyFree(&buddyVirtual, memory); }

void freePhysicalMemory(Memory memory) { buddyFree(&buddyPhysical, memory); }

void *allocVirtualMemory(U64_pow2 blockSize) {
    return buddyAllocate(&buddyVirtual, blockSize);
}

void *allocPhysicalMemory(U64_pow2 blockSize) {
    return buddyAllocate(&buddyPhysical, blockSize);
}

static void identityArrayToMappable(void_max_a *array, U32 elementSizeBytes,
                                    U64_pow2 bytesNewBuffer) {
    void *virtualBuffer = allocVirtualMemory(bytesNewBuffer);

    U64 bytesUsed = array->len * elementSizeBytes;
    U32 mapsToDo = (U32)ceilingDivide(bytesUsed, pageSizesSmallest());
    for (typeof(mapsToDo) i = 0; i < mapsToDo; i++) {
        (void)handlePageFault((U64)virtualBuffer + (i * pageSizesSmallest()));
    }

    memcpy(virtualBuffer, array->buf, array->len * elementSizeBytes);
    array->buf = virtualBuffer;
    array->cap = (U32)(bytesNewBuffer / elementSizeBytes);
}

static void pointersUpdate(U64 originalBufferLocation,
                           NodeAllocator *nodeAllocator, void **tree) {
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
        *tree = (((U8 *)*tree) + nodesBias);
    }

    for (typeof(nodeAllocator->nodesFreeList.len) i = 0;
         i < nodeAllocator->nodesFreeList.len; i++) {
        nodeAllocator->nodesFreeList.buf[i] =
            (RedBlackNodeBasic *)((U8 *)nodeAllocator->nodesFreeList.buf[i] +
                                  nodesBias);
    }
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;

static void treeWithFreeListToMappable(NodeAllocator *nodeAllocator,
                                       void **tree) {
    U64 originalBufferLocation = (U64)nodeAllocator->nodes.buf;
    identityArrayToMappable((void_max_a *)&nodeAllocator->nodes,
                            nodeAllocator->elementSizeBytes,
                            ALLOCATOR_MAX_BUFFER_SIZE);

    pointersUpdate(originalBufferLocation, nodeAllocator, tree);

    identityArrayToMappable(
        (void_max_a *)&nodeAllocator->nodesFreeList,
        sizeof(*nodeAllocator->nodesFreeList.buf),
        ALLOCATOR_MAX_BUFFER_SIZE /
            (ceilingPowerOf2(nodeAllocator->elementSizeBytes /
                             sizeof(*nodeAllocator->nodesFreeList.buf))));
}

void initMemoryManagers(KernelMemory *kernelMemory) {
    buddyPhysical.data = kernelMemory->buddyPhysical;
    if (setjmp(buddyPhysical.memoryExhausted)) {
        interruptNoMorePhysicalMemory();
    }
    if (setjmp(buddyPhysical.backingBufferExhausted)) {
        interruptNoMoreBuffer();
    }

    buddyVirtual.data = kernelMemory->buddyVirtual;
    if (setjmp(buddyVirtual.memoryExhausted)) {
        interruptNoMoreVirtualMemory();
    }
    if (setjmp(buddyVirtual.backingBufferExhausted)) {
        interruptNoMoreBuffer();
    }

    memoryMapperSizes = kernelMemory->memoryMapperSizes;

    treeWithFreeListToMappable(&memoryMapperSizes.nodeAllocator,
                               (void **)&memoryMapperSizes.tree);
}
