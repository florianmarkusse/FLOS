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

static void identityArrayToIdentity(void_max_a *array, U32 elementSizeBytes,
                                    U64_pow2 bytesNewBuffer) {
    void *bufferNew = allocPhysicalMemory(bytesNewBuffer);

    memcpy(bufferNew, array->buf, array->len * elementSizeBytes);
    array->buf = bufferNew;
    array->cap = (U32)(bytesNewBuffer / elementSizeBytes);
}

static void pointersUpdate(U64 originalBufferLocation,
                           NodeAllocator *nodeAllocator, void **trees,
                           U32 treesLen) {
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
}

static constexpr auto ALLOCATOR_MAX_BUFFER_SIZE = 1 * GiB;

static void treeWithFreeListToMappable(NodeAllocator *nodeAllocator,
                                       void **trees, U32 treesLen) {
    U64 originalBufferLocation = (U64)nodeAllocator->nodes.buf;
    identityArrayToMappable((void_max_a *)&nodeAllocator->nodes,
                            nodeAllocator->elementSizeBytes,
                            ALLOCATOR_MAX_BUFFER_SIZE);

    pointersUpdate(originalBufferLocation, nodeAllocator, trees, treesLen);

    identityArrayToMappable(
        (void_max_a *)&nodeAllocator->nodesFreeList,
        sizeof(*nodeAllocator->nodesFreeList.buf),
        ALLOCATOR_MAX_BUFFER_SIZE /
            (ceilingPowerOf2(nodeAllocator->elementSizeBytes /
                             sizeof(*nodeAllocator->nodesFreeList.buf))));
}

static void treeWithFreeListToIdentity(NodeAllocator *nodeAllocator,
                                       U64_pow2 bytesNewIdentity, void **trees,
                                       U32 treesLen) {
    U64 originalBufferLocation = (U64)nodeAllocator->nodes.buf;
    identityArrayToIdentity((void_max_a *)&nodeAllocator->nodes,
                            nodeAllocator->elementSizeBytes, bytesNewIdentity);

    pointersUpdate(originalBufferLocation, nodeAllocator, trees, treesLen);

    identityArrayToIdentity(
        (void_max_a *)&nodeAllocator->nodesFreeList,
        sizeof(*nodeAllocator->nodesFreeList.buf),
        bytesNewIdentity /
            (ceilingPowerOf2(nodeAllocator->elementSizeBytes /
                             sizeof(*nodeAllocator->nodesFreeList.buf))));
}

static constexpr auto PHYSICAL_MEMORY_ALLOCATOR_RATIO = 256;

void initMemoryManagers(KernelMemory *kernelMemory) {
    buddyPhysical.nodeAllocator = kernelMemory->buddyPhysical.nodeAllocator;
    buddyPhysical.buddy.data = kernelMemory->buddyPhysical.data;
    if (setjmp(buddyPhysical.buddy.memoryExhausted)) {
        interruptNoMorePhysicalMemory();
    }
    if (setjmp(buddyPhysical.buddy.backingBufferExhausted)) {
        interruptNoMoreBuffer();
    }

    buddyVirtual.nodeAllocator = kernelMemory->buddyVirtual.nodeAllocator;
    buddyVirtual.buddy.data = kernelMemory->buddyVirtual.data;
    if (setjmp(buddyVirtual.buddy.memoryExhausted)) {
        interruptNoMoreVirtualMemory();
    }
    if (setjmp(buddyVirtual.buddy.backingBufferExhausted)) {
        interruptNoMoreBuffer();
    }

    memoryMapperSizes = kernelMemory->memoryMapperSizes;

    // NOTE: Adding one extra map here because we are doing page faults manually
    // which will increase the physical memory usage
    treeWithFreeListToIdentity(&buddyPhysical.nodeAllocator,
                               floorPowerOf2(kernelMemory->physicalMemoryTotal /
                                             PHYSICAL_MEMORY_ALLOCATOR_RATIO),
                               (void **)&buddyPhysical.buddy.data.blocksFree,
                               buddyOrderCount(&buddyPhysical.buddy));
    treeWithFreeListToMappable(&buddyVirtual.nodeAllocator,
                               (void **)&buddyVirtual.buddy.data.blocksFree,
                               buddyOrderCount(&buddyVirtual.buddy));
    treeWithFreeListToMappable(&memoryMapperSizes.nodeAllocator,
                               (void **)&memoryMapperSizes.tree, 1);
}
