#include "shared/trees/red-black/tests/assert-memory-manager.h"
#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/trees/red-black/tests/assert.h"
#include "shared/types/array.h"

typedef struct {
    Memory memory;
    U32 index;
} NodeIndexMemory;

typedef ARRAY(NodeIndexMemory) NodeIndexMemory_a;

static void inOrderTraversalFillValues(MMTreeWithFreeList *treeWithFreeList,
                                       U32 node, NodeIndexMemory_a *values) {
    if (!node) {
        return;
    }

    RedBlackNode *treeNode =
        getNode((TreeWithFreeList *)treeWithFreeList, node);

    inOrderTraversalFillValues(
        treeWithFreeList, childNodePointerGet(treeNode, RB_TREE_LEFT), values);
    values->buf[values->len] = (NodeIndexMemory){
        .index = node,
        .memory = getMMNode(treeWithFreeList, node)->data.memory};
    values->len++;
    inOrderTraversalFillValues(
        treeWithFreeList, childNodePointerGet(treeNode, RB_TREE_RIGHT), values);
}

static void appendExpectedValues(Memory_max_a expectedValues) {
    INFO(STRING("Expected values:\n"));
    for (U32 i = 0; i < expectedValues.len; i++) {
        INFO(STRING("start: "));
        INFO(expectedValues.buf[i].start);
        INFO(STRING(" bytes: "));
        INFO(expectedValues.buf[i].bytes, .flags = NEWLINE);
    }
}

static void appendExpectedValuesAndTreeValues(Memory_max_a expectedValues,
                                              NodeIndexMemory_a inOrderValues) {
    appendExpectedValues(expectedValues);
    INFO(STRING("Red-Black Tree values:\n"));
    for (U32 i = 0; i < inOrderValues.len; i++) {
        INFO(STRING("start: "));
        INFO(inOrderValues.buf[i].memory.start);
        INFO(STRING(" bytes: "));
        INFO(inOrderValues.buf[i].memory.bytes, .flags = NEWLINE);
    }
    INFO(STRING("\n"));
}

static void assertIsBSTWitExpectedValues(MMTreeWithFreeList *treeWithFreeList,
                                         U32 nodes, Memory_max_a expectedValues,
                                         Arena scratch) {
    NodeIndexMemory_a inOrderValues = {
        .buf = NEW(&scratch, NodeIndexMemory, .count = nodes), .len = 0};

    inOrderTraversalFillValues(treeWithFreeList, treeWithFreeList->rootIndex,
                               &inOrderValues);

    if (inOrderValues.len != expectedValues.len) {
        TEST_FAILURE {
            INFO(STRING("The Red-Black Tree does not contain all the values it "
                        "should contain or it contains more!\n"));
            appendExpectedValuesAndTreeValues(expectedValues, inOrderValues);
            appendRedBlackTreeWithBadNode((TreeWithFreeList *)treeWithFreeList,
                                          0, RED_BLACK_MEMORY_MANAGER);
        }
    }

    for (U32 i = 0; i < expectedValues.len; i++) {
        bool found = false;
        for (U32 j = 0; j < inOrderValues.len; j++) {
            if (inOrderValues.buf[j].memory.start ==
                expectedValues.buf[i].start) {
                found = true;
                break;
            }
        }

        if (!found) {
            TEST_FAILURE {
                INFO(STRING("The Red-Black Tree does not contain the value "));
                INFO(expectedValues.buf[i].start, .flags = NEWLINE);
                appendExpectedValuesAndTreeValues(expectedValues,
                                                  inOrderValues);
                appendRedBlackTreeWithBadNode(
                    (TreeWithFreeList *)treeWithFreeList, 0,
                    RED_BLACK_MEMORY_MANAGER);
            }
        }
    }

    U64 previousStart = 0;
    for (U32 i = 0; i < inOrderValues.len; i++) {
        if (previousStart > inOrderValues.buf[i].memory.start) {
            TEST_FAILURE {
                INFO(STRING("Not a Binary Search Tree!\n"));
                appendRedBlackTreeWithBadNode(
                    (TreeWithFreeList *)treeWithFreeList,
                    inOrderValues.buf[i].index, RED_BLACK_MEMORY_MANAGER);
            }
        }
        previousStart = inOrderValues.buf[i].memory.start;
    }
}

static U64 mostBytes(MMTreeWithFreeList *treeWithFreeList, U32 node) {
    if (!node) {
        return 0;
    }

    MMNode *treeNode = getMMNode(treeWithFreeList, node);
    return MAX(
        treeNode->data.memory.bytes,
        mostBytes(treeWithFreeList,
                  childNodePointerGet(&treeNode->header, RB_TREE_LEFT)),
        mostBytes(treeWithFreeList,
                  childNodePointerGet(&treeNode->header, RB_TREE_RIGHT)));
}

static void
assertCorrectMostBytesInSubtreeValue(MMTreeWithFreeList *treeWithFreeList,
                                     U32 node) {
    if (!node) {
        return;
    }

    MMNode *theNode = getMMNode(treeWithFreeList, node);
    if (theNode->data.mostBytesInSubtree != mostBytes(treeWithFreeList, node)) {
        TEST_FAILURE {
            INFO(STRING("Found wrong most bytes value!\n"));
            appendRedBlackTreeWithBadNode((TreeWithFreeList *)treeWithFreeList,
                                          node, RED_BLACK_MEMORY_MANAGER);
        }
    }

    assertCorrectMostBytesInSubtreeValue(
        treeWithFreeList, childNodePointerGet(&theNode->header, RB_TREE_LEFT));
    assertCorrectMostBytesInSubtreeValue(
        treeWithFreeList, childNodePointerGet(&theNode->header, RB_TREE_LEFT));
}

static void assertCorrectMostBytesTree(MMTreeWithFreeList *treeWithFreeList) {
    assertCorrectMostBytesInSubtreeValue(treeWithFreeList,
                                         treeWithFreeList->rootIndex);
}

static void assertPrevNodeSmaller(MMTreeWithFreeList *treeWithFreeList,
                                  U32 node, U64 *prevEnd) {
    if (!node) {
        return;
    }

    MMNode *theNode = getMMNode(treeWithFreeList, node);

    assertPrevNodeSmaller(treeWithFreeList,
                          childNodePointerGet(&theNode->header, RB_TREE_LEFT),
                          prevEnd);

    if (*prevEnd && theNode->data.memory.start <= *prevEnd) {
        TEST_FAILURE {
            INFO(STRING("Should not have been inserted as a separate node!\n"));
            appendRedBlackTreeWithBadNode((TreeWithFreeList *)treeWithFreeList,
                                          node, RED_BLACK_MEMORY_MANAGER);
        }
    }

    *prevEnd = theNode->data.memory.start + theNode->data.memory.bytes;

    assertPrevNodeSmaller(treeWithFreeList,
                          childNodePointerGet(&theNode->header, RB_TREE_RIGHT),
                          prevEnd);
}

static void assertNoNodeOverlap(MMTreeWithFreeList *treeWithFreeList) {
    U64 startValue = 0;
    assertPrevNodeSmaller(treeWithFreeList, treeWithFreeList->rootIndex,
                          &startValue);
}

void assertMMRedBlackTreeValid(MMTreeWithFreeList *treeWithFreeList,
                               Memory_max_a expectedValues, Arena scratch) {
    if (!treeWithFreeList->rootIndex) {
        if (expectedValues.len == 0) {
            return;
        }
        TEST_FAILURE {
            INFO(STRING(
                "The Red-Black Tree is empty while expected values is not!\n"));
            appendExpectedValues(expectedValues);
            INFO(STRING("\n"));
        }
    }

    U32 nodes = nodeCount((TreeWithFreeList *)treeWithFreeList,
                          RED_BLACK_MEMORY_MANAGER);

    assertNoNodeOverlap(treeWithFreeList);

    assertIsBSTWitExpectedValues(treeWithFreeList, nodes, expectedValues,
                                 scratch);
    assertNoRedNodeHasRedChild((TreeWithFreeList *)treeWithFreeList, nodes,
                               RED_BLACK_MEMORY_MANAGER, scratch);
    assertPathsFromNodeHaveSameBlackHeight((TreeWithFreeList *)treeWithFreeList,
                                           nodes, RED_BLACK_MEMORY_MANAGER,
                                           scratch);
    assertCorrectMostBytesTree(treeWithFreeList);
}
