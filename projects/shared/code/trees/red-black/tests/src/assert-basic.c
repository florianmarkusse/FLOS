#include "shared/trees/red-black/tests/assert-basic.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/trees/red-black/tests/assert.h"
#include "shared/types/array-types.h"

static void inOrderTraversalFillValues(RedBlackNodeBasic *node,
                                       RedBlackNodeBasicPtr_a *values) {
    if (!node) {
        return;
    }

    inOrderTraversalFillValues(node->children[RB_TREE_LEFT], values);
    values->buf[values->len] = node;
    values->len++;
    inOrderTraversalFillValues(node->children[RB_TREE_RIGHT], values);
}

static void
appendExpectedValuesAndTreeValues(U64_max_a expectedValues,
                                  RedBlackNodeBasicPtr_a inOrderValues) {
    INFO(STRING("Expected values:\n"));
    for (U64 i = 0; i < expectedValues.len; i++) {
        INFO(expectedValues.buf[i]);
        INFO(STRING(" "));
    }
    INFO(STRING("\nRed-Black Tree values:\n"));
    for (U64 i = 0; i < inOrderValues.len; i++) {
        INFO(inOrderValues.buf[i]->value);
        INFO(STRING(" "));
    }
    INFO(STRING("\n"));
}

static void assertIsBSTWitExpectedValues(RedBlackNodeBasic *node, U64 nodes,
                                         U64_max_a expectedValues,
                                         Arena scratch) {
    RedBlackNodeBasic **_buffer =
        NEW(&scratch, RedBlackNodeBasic *, .count = nodes);
    RedBlackNodeBasicPtr_a inOrderValues = {.buf = _buffer, .len = 0};

    inOrderTraversalFillValues(node, &inOrderValues);

    if (inOrderValues.len != expectedValues.len) {
        TEST_FAILURE {
            INFO(STRING("The Red-Black Tree does not contain all the values it "
                        "should contain or it contains more!\n"));
            appendExpectedValuesAndTreeValues(expectedValues, inOrderValues);
            appendRedBlackTreeWithBadNode((RedBlackNode *)node, nullptr,
                                          RED_BLACK_BASIC);
        }
    }

    for (U64 i = 0; i < expectedValues.len; i++) {
        bool found = false;
        for (U64 j = 0; j < inOrderValues.len; j++) {
            if (inOrderValues.buf[j]->value == expectedValues.buf[i]) {
                found = true;
                break;
            }
        }

        if (!found) {
            TEST_FAILURE {
                INFO(STRING("The Red-Black Tree does not contain the value "));
                INFO(expectedValues.buf[i], NEWLINE);
                appendExpectedValuesAndTreeValues(expectedValues,
                                                  inOrderValues);
                appendRedBlackTreeWithBadNode((RedBlackNode *)node, nullptr,
                                              RED_BLACK_BASIC);
            }
        }
    }

    U64 previous = 0;
    for (U64 i = 0; i < inOrderValues.len; i++) {
        if (previous > inOrderValues.buf[i]->value) {
            TEST_FAILURE {
                INFO(STRING("Not a Binary Search Tree!\n"));
                appendRedBlackTreeWithBadNode(
                    (RedBlackNode *)node,
                    (RedBlackNode *)(inOrderValues.buf[i]), RED_BLACK_BASIC);
            }
        }
        previous = inOrderValues.buf[i]->value;
    }
}

void assertBasicRedBlackTreeValid(RedBlackNodeBasic *tree,
                                  U64_max_a expectedValues, Arena scratch) {
    if (!tree) {
        return;
    }

    U64 nodes = nodeCount((RedBlackNode *)tree, RED_BLACK_BASIC);

    assertIsBSTWitExpectedValues(tree, nodes, expectedValues, scratch);
    assertNoRedNodeHasRedChild((RedBlackNode *)tree, nodes, RED_BLACK_BASIC,
                               scratch);
    assertPathsFromNodeHaveSameBlackHeight((RedBlackNode *)tree, nodes,
                                           RED_BLACK_BASIC, scratch);
}
