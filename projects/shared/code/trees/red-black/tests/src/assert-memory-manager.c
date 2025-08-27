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

static void inOrderTraversalFillValues(NodeLocation *nodeLocation, U32 node,
                                       NodeIndexMemory_a *values) {
    if (!node) {
        return;
    }

    RedBlackNode *treeNode = getNode(nodeLocation, node);

    inOrderTraversalFillValues(nodeLocation, treeNode->children[RB_TREE_LEFT],
                               values);
    values->buf[values->len] = (NodeIndexMemory){
        .index = node, .memory = getMMNode(nodeLocation, node)->data.memory};
    values->len++;
    inOrderTraversalFillValues(nodeLocation, treeNode->children[RB_TREE_RIGHT],
                               values);
}

static void appendExpectedValuesAndTreeValues(Memory_max_a expectedValues,
                                              NodeIndexMemory_a inOrderValues) {
    INFO(STRING("Expected values:\n"));
    for (U32 i = 0; i < expectedValues.len; i++) {
        INFO(STRING("start: "));
        INFO(expectedValues.buf[i].start);
        INFO(STRING(" bytes: "));
        INFO(expectedValues.buf[i].bytes, .flags = NEWLINE);
    }
    INFO(STRING("Red-Black Tree values:\n"));
    for (U32 i = 0; i < inOrderValues.len; i++) {
        INFO(STRING("start: "));
        INFO(inOrderValues.buf[i].memory.start);
        INFO(STRING(" bytes: "));
        INFO(inOrderValues.buf[i].memory.bytes, .flags = NEWLINE);
    }
    INFO(STRING("\n"));
}

static void assertIsBSTWitExpectedValues(NodeLocation *nodeLocation, U32 node,
                                         U32 nodes, Memory_max_a expectedValues,
                                         Arena scratch) {
    NodeIndexMemory_a inOrderValues = {
        .buf = NEW(&scratch, NodeIndexMemory, .count = nodes), .len = 0};

    inOrderTraversalFillValues(nodeLocation, node, &inOrderValues);

    if (inOrderValues.len != expectedValues.len) {
        TEST_FAILURE {
            INFO(STRING("The Red-Black Tree does not contain all the values it "
                        "should contain or it contains more!\n"));
            appendExpectedValuesAndTreeValues(expectedValues, inOrderValues);
            appendRedBlackTreeWithBadNode(nodeLocation, node, 0,
                                          RED_BLACK_MEMORY_MANAGER);
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
                appendRedBlackTreeWithBadNode(nodeLocation, node, 0,
                                              RED_BLACK_MEMORY_MANAGER);
            }
        }
    }

    U64 previousStart = 0;
    for (U32 i = 0; i < inOrderValues.len; i++) {
        if (previousStart > inOrderValues.buf[i].memory.start) {
            TEST_FAILURE {
                INFO(STRING("Not a Binary Search Tree!\n"));
                appendRedBlackTreeWithBadNode(nodeLocation, node,
                                              inOrderValues.buf[i].index,
                                              RED_BLACK_MEMORY_MANAGER);
            }
        }
        previousStart = inOrderValues.buf[i].memory.start;
    }
}

static U64 mostBytes(NodeLocation *nodeLocation, U32 node) {
    if (!node) {
        return 0;
    }

    MMNode *treeNode = getMMNode(nodeLocation, node);
    return MAX(
        treeNode->data.memory.bytes,
        mostBytes(nodeLocation, treeNode->header.children[RB_TREE_LEFT]),
        mostBytes(nodeLocation, treeNode->header.children[RB_TREE_RIGHT]));
}

static void assertCorrectMostBytesInSubtreeValue(NodeLocation *nodeLocation,
                                                 U32 tree, U32 node) {
    if (!node) {
        return;
    }

    MMNode *theNode = getMMNode(nodeLocation, node);
    if (theNode->data.mostBytesInSubtree != mostBytes(nodeLocation, node)) {
        TEST_FAILURE {
            INFO(STRING("Found wrong most bytes value!\n"));
            appendRedBlackTreeWithBadNode(nodeLocation, tree, node,
                                          RED_BLACK_MEMORY_MANAGER);
        }
    }

    assertCorrectMostBytesInSubtreeValue(
        nodeLocation, tree, theNode->header.children[RB_TREE_LEFT]);
    assertCorrectMostBytesInSubtreeValue(
        nodeLocation, tree, theNode->header.children[RB_TREE_RIGHT]);
}

static void assertCorrectMostBytesTree(NodeLocation *nodeLocation, U32 tree) {
    assertCorrectMostBytesInSubtreeValue(nodeLocation, tree, tree);
}

static void assertPrevNodeSmaller(NodeLocation *nodeLocation, U32 tree,
                                  U32 node, U64 *prevEnd) {
    if (!node) {
        return;
    }

    MMNode *theNode = getMMNode(nodeLocation, node);

    assertPrevNodeSmaller(nodeLocation, tree,
                          theNode->header.children[RB_TREE_LEFT], prevEnd);

    if (*prevEnd && theNode->data.memory.start <= *prevEnd) {
        TEST_FAILURE {
            INFO(STRING("Should not have been inserted as a separate node!\n"));
            appendRedBlackTreeWithBadNode(nodeLocation, tree, node,
                                          RED_BLACK_MEMORY_MANAGER);
        }
    }

    *prevEnd = theNode->data.memory.start + theNode->data.memory.bytes;

    assertPrevNodeSmaller(nodeLocation, tree,
                          theNode->header.children[RB_TREE_RIGHT], prevEnd);
}

static void assertNoNodeOverlap(NodeLocation *nodeLocation, U32 tree) {
    U64 startValue = 0;
    assertPrevNodeSmaller(nodeLocation, tree, tree, &startValue);
}

void assertMMRedBlackTreeValid(NodeLocation *nodeLocation, U32 tree,
                               Memory_max_a expectedValues, Arena scratch) {
    if (!tree) {
        return;
    }

    U32 nodes = nodeCount(nodeLocation, tree, RED_BLACK_MEMORY_MANAGER);

    assertNoNodeOverlap(nodeLocation, tree);

    assertIsBSTWitExpectedValues(nodeLocation, tree, nodes, expectedValues,
                                 scratch);
    assertNoRedNodeHasRedChild(nodeLocation, tree, nodes,
                               RED_BLACK_MEMORY_MANAGER, scratch);
    assertPathsFromNodeHaveSameBlackHeight(nodeLocation, tree, nodes,
                                           RED_BLACK_MEMORY_MANAGER, scratch);
    assertCorrectMostBytesTree(nodeLocation, tree);
}
