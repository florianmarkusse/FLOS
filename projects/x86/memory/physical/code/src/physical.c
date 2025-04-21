#include "x86/memory/physical.h"

#include "shared/memory/sizes.h"
#include "shared/trees/red-black.h"
#include "x86/memory/definitions.h"

#include "abstraction/interrupts.h"
#include "abstraction/jmp.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/kernel-parameters.h" // for KernelMemory
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h" // for U64, U32, U8

RedBlackNode *tree;
static Arena allocatable;
static RedBlackNodePtr_a freeList;

// The remaining bytes are put back into the red black tree or we have a perfect
// match and the node's memory location can be now put in the free list
static void handleFreeMemory(RedBlackNode *availableMemory, U64 bytes) {
    availableMemory->memory.bytes -= bytes;
    if (availableMemory->memory.bytes) {
        availableMemory->memory.start += bytes;
        insertRedBlackNode(&tree, availableMemory);
    } else {
        freeList.buf[freeList.len] = availableMemory;
        freeList.len++;
    }
}

static RedBlackNode *getMemoryNode(U64 bytes) {
    RedBlackNode *availableMemory = deleteAtLeastRedBlackNode(&tree, bytes);
    if (!availableMemory) {
        interruptTooLargeAllocation();
    }
    return availableMemory;
}

void freeMemory(Memory memory) {
    RedBlackNode *newNode;
    if (freeList.len > 0) {
        newNode = freeList.buf[freeList.len - 1];
        freeList.len--;
    } else {
        newNode = NEW(&allocatable, RedBlackNode);
    }

    newNode->memory = memory;
    insertRedBlackNode(&tree, newNode);
}

void *allocPhysicalMemory(U64 bytes) {
    RedBlackNode *availableMemory = getMemoryNode(bytes);
    void *result = (void *)availableMemory->memory.start;
    handleFreeMemory(availableMemory, bytes);

    return result;
}

// TODO: Make this architecture agnostic?
U64 getPageForMappingVirtualMemory() {
    // TODO: THIS!
    return (U64)allocPhysicalMemory(X86_4KIB_PAGE);
}

// NOTE: Coming into this, All the memory is identity mapped. Having to do some
// boostrapping here.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    tree = kernelMemory.tree;

    allocatable = kernelMemory.allocator;
    if (setjmp(allocatable.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }

    U64 redBlackNodesPossibleInAllocator =
        (kernelMemory.allocator.end - kernelMemory.allocator.beg) /
        sizeof(*tree);
    U64 freeListRequiredSize = redBlackNodesPossibleInAllocator * sizeof(tree);

    RedBlackNode *availableMemory = getMemoryNode(freeListRequiredSize);
    freeList = (RedBlackNodePtr_a){
        .len = 0, .buf = (RedBlackNode **)availableMemory->memory.start};
    handleFreeMemory(availableMemory, freeListRequiredSize);
}
