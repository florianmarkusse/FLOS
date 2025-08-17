#include "shared/trees/red-black/tests/assert-memory-manager.h"
#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/trees/red-black/tests/assert.h"
#include "shared/types/array.h"

typedef ARRAY(MMNode *) MMNodePtr_a;

static void appendExpectedValuesAndTreeValues(Memory_max_a expectedValues,
                                              MMNodePtr_a inOrderValues) {
    INFO(STRING("Expected values:\n"));
    for (U32 i = 0; i < expectedValues.len; i++) {
        INFO(STRING("start: "));
        INFO(expectedValues.buf[i].start);
        INFO(STRING(" bytes: "));
        INFO(expectedValues.buf[i].bytes, NEWLINE);
    }
    INFO(STRING("Red-Black Tree values:\n"));
    for (U32 i = 0; i < inOrderValues.len; i++) {
        INFO(STRING("start: "));
        INFO(inOrderValues.buf[i]->memory.start);
        INFO(STRING(" bytes: "));
        INFO(inOrderValues.buf[i]->memory.bytes, NEWLINE);
    }
    INFO(STRING("\n"));
}

static void inOrderTraversalFillValues(MMNode *node, MMNodePtr_a *values) {
    if (!node) {
        return;
    }

    inOrderTraversalFillValues(node->children[RB_TREE_LEFT], values);
    values->buf[values->len] = node;
    values->len++;
    inOrderTraversalFillValues(node->children[RB_TREE_RIGHT], values);
}

static void assertIsBSTWitExpectedValues(MMNode *node, U32 nodes,
                                         Memory_max_a expectedValues,
                                         Arena scratch) {
    MMNode **_buffer = NEW(&scratch, MMNode *, nodes);
    MMNodePtr_a inOrderValues = {.buf = _buffer, .len = 0};

    inOrderTraversalFillValues(node, &inOrderValues);

    if (inOrderValues.len != expectedValues.len) {
        TEST_FAILURE {
            INFO(STRING("The Red-Black Tree does not contain all the values it "
                        "should contain or it contains more!\n"));
            appendExpectedValuesAndTreeValues(expectedValues, inOrderValues);
            appendRedBlackTreeWithBadNode((RedBlackNode *)node, nullptr,
                                          RED_BLACK_MEMORY_MANAGER);
        }
    }

    for (U32 i = 0; i < expectedValues.len; i++) {
        bool found = false;
        for (U32 j = 0; j < inOrderValues.len; j++) {
            if (inOrderValues.buf[j]->memory.start ==
                expectedValues.buf[i].start) {
                found = true;
                break;
            }
        }

        if (!found) {
            TEST_FAILURE {
                INFO(STRING("The Red-Black Tree does not contain the value "));
                INFO(expectedValues.buf[i].start, NEWLINE);
                appendExpectedValuesAndTreeValues(expectedValues,
                                                  inOrderValues);
                appendRedBlackTreeWithBadNode((RedBlackNode *)node, nullptr,
                                              RED_BLACK_MEMORY_MANAGER);
            }
        }
    }

    U64 previousStart = 0;
    for (U32 i = 0; i < inOrderValues.len; i++) {
        if (previousStart > inOrderValues.buf[i]->memory.start) {
            TEST_FAILURE {
                INFO(STRING("Not a Binary Search Tree!\n"));
                appendRedBlackTreeWithBadNode(
                    (RedBlackNode *)node,
                    (RedBlackNode *)(inOrderValues.buf[i]),
                    RED_BLACK_MEMORY_MANAGER);
            }
        }
        previousStart = inOrderValues.buf[i]->memory.start;
    }
}

static U64 mostBytes(MMNode *node) {
    if (!node) {
        return 0;
    }

    return MAX(node->memory.bytes, mostBytes(node->children[RB_TREE_LEFT]),
               mostBytes(node->children[RB_TREE_RIGHT]));
}

static void assertCorrectMostBytesInSubtreeValue(MMNode *tree, MMNode *node) {
    if (!node) {
        return;
    }

    if (node->mostBytesInSubtree != mostBytes(node)) {
        TEST_FAILURE {
            INFO(STRING("Found wrong most bytes value!\n"));
            appendRedBlackTreeWithBadNode((RedBlackNode *)tree,
                                          (RedBlackNode *)node,
                                          RED_BLACK_MEMORY_MANAGER);
        }
    }

    assertCorrectMostBytesInSubtreeValue(tree, node->children[RB_TREE_LEFT]);
    assertCorrectMostBytesInSubtreeValue(tree, node->children[RB_TREE_RIGHT]);
}

static void assertCorrectMostBytesTree(MMNode *tree) {
    assertCorrectMostBytesInSubtreeValue(tree, tree);
}

static void assertPrevNodeSmaller(MMNode *tree, MMNode *node, U64 *prevEnd) {
    if (!node) {
        return;
    }

    assertPrevNodeSmaller(tree, node->children[RB_TREE_LEFT], prevEnd);

    if (*prevEnd && node->memory.start <= *prevEnd) {
        TEST_FAILURE {
            INFO(STRING("Should not have been inserted as a separate node!\n"));
            appendRedBlackTreeWithBadNode((RedBlackNode *)tree,
                                          (RedBlackNode *)node,
                                          RED_BLACK_MEMORY_MANAGER);
        }
    }

    *prevEnd = node->memory.start + node->memory.bytes;

    assertPrevNodeSmaller(tree, node->children[RB_TREE_RIGHT], prevEnd);
}

static void assertNoNodeOverlap(MMNode *tree) {
    U64 startValue = 0;
    assertPrevNodeSmaller(tree, tree, &startValue);
}

void assertMMRedBlackTreeValid(MMNode *tree, Memory_max_a expectedValues,
                               Arena scratch) {
    if (!tree) {
        return;
    }

    U32 nodes = nodeCount((RedBlackNode *)tree, RED_BLACK_MEMORY_MANAGER);

    assertNoNodeOverlap(tree);

    assertIsBSTWitExpectedValues(tree, nodes, expectedValues, scratch);
    assertNoRedNodeHasRedChild((RedBlackNode *)tree, nodes,
                               RED_BLACK_MEMORY_MANAGER, scratch);
    assertPathsFromNodeHaveSameBlackHeight((RedBlackNode *)tree, nodes,
                                           RED_BLACK_MEMORY_MANAGER, scratch);
    assertCorrectMostBytesTree(tree);
}
