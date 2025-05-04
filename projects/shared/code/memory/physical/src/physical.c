#include "shared/memory/physical.h"
#include "abstraction/interrupts.h"
#include "abstraction/memory/physical.h"

// NOTE: Not multi threaded safe!

RedBlackNodeMM *physicalTree;
static Arena allocatable;
static RedBlackNodeMMPtr_a freeList;

// The remaining bytes are put back into the red black tree or we have a perfect
// match and the node's memory location can be now put in the free list
static void handleFreeMemory(RedBlackNodeMM *availableMemory, U64 bytes) {
    availableMemory->memory.bytes -= bytes;
    if (availableMemory->memory.bytes) {
        availableMemory->memory.start += bytes;
        insertRedBlackNodeMM(&physicalTree, availableMemory);
    } else {
        freeList.buf[freeList.len] = availableMemory;
        freeList.len++;
    }
}

static RedBlackNodeMM *getMemoryNode(U64 bytes) {
    RedBlackNodeMM *availableMemory =
        deleteAtLeastRedBlackNodeMM(&physicalTree, bytes);
    if (!availableMemory) {
        interruptTooLargeAllocation();
    }
    return availableMemory;
}

void freeMemory(Memory memory) {
    RedBlackNodeMM *newNode;
    if (freeList.len > 0) {
        newNode = freeList.buf[freeList.len - 1];
        freeList.len--;
    } else {
        newNode = NEW(&allocatable, RedBlackNodeMM);
    }
    newNode->memory = memory;

    InsertResult insertResult = insertRedBlackNodeMM(&physicalTree, newNode);
    for (U64 i = 0; (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
                    insertResult.freed[i];
         i++) {
        freeList.buf[freeList.len] = insertResult.freed[i];
        freeList.len++;
    }
}

void *allocPhysicalMemory(U64 bytes) {
    RedBlackNodeMM *availableMemory = getMemoryNode(bytes);
    void *result = (void *)availableMemory->memory.start;
    handleFreeMemory(availableMemory, bytes);
    return result;
}

U64 getPageForMappingVirtualMemory(U64 pageSize) {
    return (U64)allocPhysicalMemory(pageSize);
}

// NOTE: Coming into this, All the memory is identity mapped. Having to do some
// boostrapping here.
void initPhysicalMemoryManager(MemoryTree physicalMemoryTree) {
    physicalTree = physicalMemoryTree.tree;

    allocatable = physicalMemoryTree.allocator;
    if (setjmp(allocatable.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }

    U64 redBlackNodeMMsPossibleInAllocator =
        (physicalMemoryTree.allocator.end - physicalMemoryTree.allocator.beg) /
        sizeof(*physicalTree);
    U64 freeListRequiredSize =
        redBlackNodeMMsPossibleInAllocator * sizeof(physicalTree);

    RedBlackNodeMM *availableMemory = getMemoryNode(freeListRequiredSize);
    freeList = (RedBlackNodeMMPtr_a){
        .len = 0, .buf = (RedBlackNodeMM **)availableMemory->memory.start};
    handleFreeMemory(availableMemory, freeListRequiredSize);
}
