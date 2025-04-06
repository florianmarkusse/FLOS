#include "shared/trees/red-black/tests/correct.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/trees/red-black.h"
#include "shared/types/array-types.h"

typedef ARRAY(RedBlackNode *) RedBlackNodePtr_a;

static void printTreeIndented(RedBlackNode *node, int depth, string prefix,
                              RedBlackNode *badNode) {
    if (!node) {
        return;
    }

    printTreeIndented(node->children[RIGHT_CHILD], depth + 1, STRING("R---"),
                      badNode);

    for (int i = 0; i < depth; i++) {
        INFO(STRING("    "));
    }
    INFO(prefix);
    INFO(STRING("Value: "));
    INFO(node->value);
    INFO(STRING(", Color: "));
    INFO(node->color == RED ? STRING("RED") : STRING("BLACK"));

    if (node == badNode) {
        appendColor(COLOR_RED, STDOUT);
        INFO(STRING("    !!!!    !!!!"));
        appendColorReset(STDOUT);
    }

    INFO(STRING("\n"));

    printTreeIndented(node->children[LEFT_CHILD], depth + 1, STRING("L---"),
                      badNode);
}

static void printRedBlackTree(RedBlackNode *root, RedBlackNode *badNode) {
    INFO(STRING("Red-Black Tree Structure:"), NEWLINE);
    printTreeIndented(root, 0, STRING("Root---"), badNode);
}

static void inOrderTraversal(RedBlackNode *node, RedBlackNodePtr_a *values) {
    if (!node) {
        return;
    }

    inOrderTraversal(node->children[LEFT_CHILD], values);
    values->buf[values->len] = node;
    values->len++;
    inOrderTraversal(node->children[RIGHT_CHILD], values);
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

        for (U64 i = 0; i < CHILD_COUNT; i++) {
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

static bool isBinarySearchTree(RedBlackNode *node, U64 nodes, Arena scratch) {
    RedBlackNode **_buffer = NEW(&scratch, RedBlackNode *, nodes);
    RedBlackNodePtr_a inOrderValues = {.buf = _buffer, .len = 0};

    inOrderTraversal(node, &inOrderValues);

    U64 previous = 0;
    for (U64 i = 0; i < inOrderValues.len; i++) {
        if (previous > inOrderValues.buf[i]->value) {
            TEST_FAILURE {
                INFO(STRING("Not a Binary Search Tree!\n"));
                printRedBlackTree(node, inOrderValues.buf[i]);
            }
            return false;
        }
        previous = inOrderValues.buf[i]->value;
    }

    return true;
}

static bool redParentHasRedChild(RedBlackNode *node, U64 childIndex) {
    if (node->children[childIndex] &&
        node->children[childIndex]->color == RED) {
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

        if (node->color == RED) {
            if (redParentHasRedChild(node, LEFT_CHILD) ||
                redParentHasRedChild(node, RIGHT_CHILD)) {
                TEST_FAILURE {
                    INFO(STRING("Node has a red child!\n"));
                    printRedBlackTree(tree, node);
                }
                return true;
            }
        }

        for (U64 i = 0; i < CHILD_COUNT; i++) {
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
        if (node->color == BLACK) {
            current++;
        }

        collectBlackHeightsForEachPath(node->children[LEFT_CHILD], blackHeights,
                                       current);
        collectBlackHeightsForEachPath(node->children[RIGHT_CHILD],
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
                TEST_FAILURE {
                    INFO(STRING("Found differing black heights!\n"));
                    printRedBlackTree(tree, node);

                    INFO(STRING("Black heights calculated (left-to-right):"));
                    for (U64 j = 0; j < blackHeights.len; j++) {
                        INFO(blackHeights.buf[j]);
                        INFO(STRING(" "));
                    }
                    INFO(STRING("\n"));
                }

                return false;
            }
        }

        for (U64 i = 0; i < CHILD_COUNT; i++) {
            if (node->children[i]) {
                buffer[len] = node->children[i];
                len++;
            }
        }
    }

    return true;
}

void assertRedBlackTreeValid(RedBlackNode *tree, Arena scratch) {
    if (!tree) {
        testSuccess();
        return;
    }

    U64 nodes = nodeCount(tree);
    if (!nodes) {
        TEST_FAILURE {
            INFO(STRING("Tree has too many nodes to assert correctness!"));
            printRedBlackTree(tree, tree);
        }
    }

    if (!isBinarySearchTree(tree, nodes, scratch)) {
        return;
    }

    if (tree->color == RED) {
        TEST_FAILURE {
            INFO(STRING("Root is not black!\n"));
            printRedBlackTree(tree, tree);
        }
        return;
    }

    if (anyRedNodeHasRedChild(tree, nodes, scratch)) {
        return;
    }

    if (!pathsFromNodeHaveSameBlackHeight(tree, nodes, scratch)) {
        return;
    }

    testSuccess();
}
