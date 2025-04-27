#include "shared/trees/red-black/tests/correct.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/trees/red-black.h"
#include "shared/types/array-types.h"

static void printTreeIndented(RedBlackNode *node, int depth, string prefix,
                              RedBlackNode *badNode) {
    if (!node) {
        return;
    }

    printTreeIndented(node->children[RB_TREE_RIGHT], depth + 1, STRING("R---"),
                      badNode);

    for (int i = 0; i < depth; i++) {
        INFO(STRING("    "));
    }
    INFO(prefix);
    INFO(STRING(", Color: "));
    INFO(node->color == RB_TREE_RED ? STRING("RED") : STRING("BLACK"));
    INFO(STRING(" Bytes: "));
    INFO(node->memory.bytes);
    INFO(STRING(" Most bytes in subtree: "));
    INFO(node->mostBytesInSubtree);

    if (node == badNode) {
        appendColor(COLOR_RED, STDOUT);
        INFO(STRING("    !!!!    !!!!"));
        appendColorReset(STDOUT);
    }

    INFO(STRING("\n"));

    printTreeIndented(node->children[RB_TREE_LEFT], depth + 1, STRING("L---"),
                      badNode);
}

void appendRedBlackTreeWithBadNode(RedBlackNode *root, RedBlackNode *badNode) {
    INFO(STRING("Red-Black Tree Structure:"), NEWLINE);
    printTreeIndented(root, 0, STRING("Root---"), badNode);
}

static void inOrderTraversalFillValues(RedBlackNode *node,
                                       RedBlackNodePtr_a *values) {
    if (!node) {
        return;
    }

    inOrderTraversalFillValues(node->children[RB_TREE_LEFT], values);
    values->buf[values->len] = node;
    values->len++;
    inOrderTraversalFillValues(node->children[RB_TREE_RIGHT], values);
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
                    TEST_FAILURE {
                        INFO(STRING(
                            "Tree has too many nodes to assert correctness!"));
                        appendRedBlackTreeWithBadNode(tree, tree);
                    }
                }
                buffer[len] = node->children[i];
                len++;
            }
        }
    }

    return result;
}

static void appendExpectedValuesAndTreeValues(U64_max_a expectedValues,
                                              RedBlackNodePtr_a inOrderValues) {
    INFO(STRING("Expected values:\n"));
    for (U64 i = 0; i < expectedValues.len; i++) {
        INFO(expectedValues.buf[i]);
        INFO(STRING(" "));
    }
    INFO(STRING("\nRed-Black Tree values:\n"));
    for (U64 i = 0; i < inOrderValues.len; i++) {
        INFO(inOrderValues.buf[i]->memory.bytes);
        INFO(STRING(" "));
    }
    INFO(STRING("\n"));
}

static void assertIsBSTWitExpectedValues(RedBlackNode *node, U64 nodes,
                                         U64_max_a expectedValues,
                                         Arena scratch) {
    RedBlackNode **_buffer = NEW(&scratch, RedBlackNode *, nodes);
    RedBlackNodePtr_a inOrderValues = {.buf = _buffer, .len = 0};

    inOrderTraversalFillValues(node, &inOrderValues);

    if (inOrderValues.len != expectedValues.len) {
        TEST_FAILURE {
            INFO(STRING("The Red-Black Tree does not contain all the values it "
                        "should contain or it contains more!\n"));
            appendExpectedValuesAndTreeValues(expectedValues, inOrderValues);
            appendRedBlackTreeWithBadNode(node, nullptr);
        }
    }

    for (U64 i = 0; i < expectedValues.len; i++) {
        bool found = false;
        for (U64 j = 0; j < inOrderValues.len; j++) {
            if (inOrderValues.buf[j]->memory.bytes == expectedValues.buf[i]) {
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
                appendRedBlackTreeWithBadNode(node, nullptr);
            }
        }
    }

    U64 previous = 0;
    for (U64 i = 0; i < inOrderValues.len; i++) {
        if (previous > inOrderValues.buf[i]->memory.bytes) {
            TEST_FAILURE {
                INFO(STRING("Not a Binary Search Tree!\n"));
                appendRedBlackTreeWithBadNode(node, inOrderValues.buf[i]);
            }
        }
        previous = inOrderValues.buf[i]->memory.bytes;
    }
}

static bool redParentHasRedChild(RedBlackNode *node,
                                 RedBlackDirection direction) {
    if (node->children[direction] &&
        node->children[direction]->color == RB_TREE_RED) {
        return true;
    }

    return false;
}

static void assertNoRedNodeHasRedChild(RedBlackNode *tree, U64 nodes,
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
                TEST_FAILURE {
                    INFO(STRING("Red node has a red child!\n"));
                    appendRedBlackTreeWithBadNode(tree, node);
                }
            }
        }

        for (U64 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            if (node->children[i]) {
                buffer[len] = node->children[i];
                len++;
            }
        }
    }
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

static void assertPathsFromNodeHaveSameBlackHeight(RedBlackNode *tree,
                                                   U64 nodes, Arena scratch) {
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
                TEST_FAILURE {
                    INFO(STRING("Found differing black heights!\n"));
                    appendRedBlackTreeWithBadNode(tree, node);

                    INFO(STRING("Black heights calculated (left-to-right):"));
                    for (U64 j = 0; j < blackHeights.len; j++) {
                        INFO(blackHeights.buf[j]);
                        INFO(STRING(" "));
                    }
                    INFO(STRING("\n"));
                }
            }
        }

        for (U64 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            if (node->children[i]) {
                buffer[len] = node->children[i];
                len++;
            }
        }
    }
}

static U64 mostBytes(RedBlackNode *node) {
    if (!node) {
        return 0;
    }

    return MAX(node->memory.bytes, mostBytes(node->children[RB_TREE_LEFT]),
               mostBytes(node->children[RB_TREE_RIGHT]));
}

static void assertCorrectMostBytesInSubtreeValue(RedBlackNode *tree,
                                                 RedBlackNode *node) {
    if (!node) {
        return;
    }

    if (node->mostBytesInSubtree != mostBytes(node)) {
        TEST_FAILURE {
            INFO(STRING("Found wrong most bytes value!\n"));
            appendRedBlackTreeWithBadNode(tree, node);
        }
    }

    assertCorrectMostBytesInSubtreeValue(tree, node->children[RB_TREE_LEFT]);
    assertCorrectMostBytesInSubtreeValue(tree, node->children[RB_TREE_RIGHT]);
}

static void assertCorrectMostBytesTree(RedBlackNode *tree) {
    assertCorrectMostBytesInSubtreeValue(tree, tree);
}

void assertRedBlackTreeValid(RedBlackNode *tree, U64_max_a expectedValues,
                             Arena scratch) {
    if (!tree) {
        return;
    }

    U64 nodes = nodeCount(tree);

    assertIsBSTWitExpectedValues(tree, nodes, expectedValues, scratch);
    assertNoRedNodeHasRedChild(tree, nodes, scratch);
    assertPathsFromNodeHaveSameBlackHeight(tree, nodes, scratch);
    assertCorrectMostBytesTree(tree);
}
