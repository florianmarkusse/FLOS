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

static Arena tester;

static void inOrderTraversal(RedBlackNode *node, RedBlackNodePtr_a *values) {
    if (!node) {
        return;
    }

    inOrderTraversal(node->children[RB_TREE_LEFT], values);
    values->buf[values->len] = node;
    values->len++;
    inOrderTraversal(node->children[RB_TREE_RIGHT], values);
}

static constexpr auto MAX_TREE_HEIGHT = 256;
static U64 nodeCount(RedBlackNode *tree) {
    RedBlackNode *buffer[MAX_TREE_HEIGHT];
    U64 len = 0;

    U64 result = 0;

    buffer[len] = tree;
    len++;
    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;
        result++;

        for (U64 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            if (node->children[i]) {
                if (len > MAX_TREE_HEIGHT) {
                    return 0;
                }
                buffer[len] = node->children[i];
                len++;
            }
        }
    }

    return result;
}

static bool isBST(RedBlackNode *node, U64 nodes, Arena scratch) {
    RedBlackNode **_buffer = NEW(&scratch, RedBlackNode *, nodes);
    RedBlackNodePtr_a inOrderValues = {.buf = _buffer, .len = 0};

    inOrderTraversal(node, &inOrderValues);

    U64 previous = 0;
    for (U64 i = 0; i < inOrderValues.len; i++) {
        if (previous > inOrderValues.buf[i]->memory.bytes) {
            return false;
        }
        previous = inOrderValues.buf[i]->memory.bytes;
    }

    return true;
}

static bool redParentHasRedChild(RedBlackNode *node,
                                 RedBlackDirection direction) {
    if (node->children[direction] &&
        node->children[direction]->color == RB_TREE_RED) {
        return true;
    }

    return false;
}

static bool anyRedNodeHasRedChild(RedBlackNode *tree, U64 nodes,
                                  Arena scratch) {
    RedBlackNode **buffer = NEW(&scratch, RedBlackNode *, nodes);
    U64 len = 0;

    buffer[len] = tree;
    len++;
    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;

        if (node->color == RB_TREE_RED) {
            if (redParentHasRedChild(node, RB_TREE_LEFT) ||
                redParentHasRedChild(node, RB_TREE_RIGHT)) {
                return true;
            }
        }

        for (U64 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            if (node->children[i]) {
                buffer[len] = node->children[i];
                len++;
            }
        }
    }

    return false;
}

static void collectBlackHeightsForEachPath(RedBlackNode *node,
                                           U64_a *blackHeights, U64 current) {
    if (!node) {
        blackHeights->buf[blackHeights->len] = current + 1;
        blackHeights->len++;
    } else {
        if (node->color == RB_TREE_BLACK) {
            current++;
        }

        collectBlackHeightsForEachPath(node->children[RB_TREE_LEFT],
                                       blackHeights, current);
        collectBlackHeightsForEachPath(node->children[RB_TREE_RIGHT],
                                       blackHeights, current);
    }
}

static bool pathsFromNodeHaveSameBlackHeight(RedBlackNode *tree, U64 nodes,
                                             Arena scratch) {
    RedBlackNode **buffer = NEW(&scratch, RedBlackNode *, nodes);
    U64 len = 0;

    buffer[len] = tree;
    len++;

    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;

        U64 *_blackHeightsBuffer = NEW(&scratch, U64, nodes);
        U64_a blackHeights = {.buf = _blackHeightsBuffer, .len = 0};

        collectBlackHeightsForEachPath(node, &blackHeights, 0);

        U64 first = blackHeights.buf[0];
        for (U64 i = 1; i < blackHeights.len; i++) {
            if (blackHeights.buf[i] != first) {
                return false;
            }
        }

        for (U64 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            if (node->children[i]) {
                buffer[len] = node->children[i];
                len++;
            }
        }
    }

    return true;
}

static bool assertRedBlackTreeValid(RedBlackNode *tree, Arena scratch) {
    if (!tree) {
        return true;
    }

    U64 nodes = nodeCount(tree);
    if (!nodes) {
        return false;
    }

    if (!isBST(tree, nodes, scratch)) {
        return false;
    }

    if (anyRedNodeHasRedChild(tree, nodes, scratch)) {
        return false;
    }

    if (!pathsFromNodeHaveSameBlackHeight(tree, nodes, scratch)) {
        return false;
    }

    return true;
}

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

    //    bool thing = assertRedBlackTreeValid(tree, tester);
    //    if (!thing) {
    //        interruptNoMorePhysicalMemory();
    //    }
}

void *allocPhysicalMemory(U64 bytes) {
    RedBlackNode *availableMemory = getMemoryNode(bytes);
    void *result = (void *)availableMemory->memory.start;
    handleFreeMemory(availableMemory, bytes);

    //    bool thing = assertRedBlackTreeValid(tree, tester);
    //    if (!thing) {
    //        interruptNoMorePhysicalMemory();
    //    }

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

    void *memory = allocPhysicalMemory(1 * MiB);
    tester =
        (Arena){.beg = memory, .curFree = memory, .end = memory + (1 * MiB)};
    if (setjmp(tester.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }
}
