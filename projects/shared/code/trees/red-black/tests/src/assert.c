#include "shared/trees/red-black/tests/assert.h"
#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/trees/red-black-memory-manager.h"

static void printTreeIndented(RedBlackNode *node, int depth, string prefix,
                              RedBlackNode *badNode,
                              RedBlackTreeType treeType) {
    if (!node) {
        return;
    }

    printTreeIndented(node->children[RB_TREE_RIGHT], depth + 1, STRING("R---"),
                      badNode, treeType);

    for (int i = 0; i < depth; i++) {
        INFO(STRING("    "));
    }
    INFO(prefix);
    INFO(STRING(", Color: "));
    INFO(node->color == RB_TREE_RED ? STRING("RED") : STRING("BLACK"));

    switch (treeType) {
    case RED_BLACK_BASIC: {
        RedBlackNodeBasic *basicNode = (RedBlackNodeBasic *)node;
        INFO(STRING(" Value: "));
        INFO(basicNode->value);
        break;
    }
    case RED_BLACK_MEMORY_MANAGER: {
        RedBlackNodeMM *memoryManagerNode = (RedBlackNodeMM *)node;
        INFO(STRING(" Start: "));
        INFO((void *)memoryManagerNode->memory.start);
        INFO(STRING(" Bytes: "));
        INFO((void *)memoryManagerNode->memory.bytes);
        INFO(STRING(" Most bytes in subtree: "));
        INFO((void *)memoryManagerNode->mostBytesInSubtree);
        break;
    }
    }

    if (node == badNode) {
        appendColor(COLOR_RED, STDOUT);
        INFO(STRING("    !!!!    !!!!"));
        appendColorReset(STDOUT);
    }

    INFO(STRING("\n"));

    printTreeIndented(node->children[RB_TREE_LEFT], depth + 1, STRING("L---"),
                      badNode, treeType);
}

void appendRedBlackTreeWithBadNode(RedBlackNode *root, RedBlackNode *badNode,
                                   RedBlackTreeType treeType) {
    INFO(STRING("Red-Black Tree Structure:"), NEWLINE);
    printTreeIndented(root, 0, STRING("Root---"), badNode, treeType);
}

U64 nodeCount(RedBlackNode *tree, RedBlackTreeType treeType) {
    RedBlackNode *buffer[RB_TREE_MAX_HEIGHT];
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
                if (len > RB_TREE_MAX_HEIGHT) {
                    TEST_FAILURE {
                        INFO(STRING(
                            "Tree has too many nodes to assert correctness!"));
                        appendRedBlackTreeWithBadNode(tree, tree, treeType);
                    }
                }
                buffer[len] = node->children[i];
                len++;
            }
        }
    }

    return result;
}

static bool redParentHasRedChild(RedBlackNode *node,
                                 RedBlackDirection direction) {
    if (node->children[direction] &&
        node->children[direction]->color == RB_TREE_RED) {
        return true;
    }

    return false;
}

void assertNoRedNodeHasRedChild(RedBlackNode *tree, U64 nodes,
                                RedBlackTreeType treeType, Arena scratch) {
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
                    appendRedBlackTreeWithBadNode(tree, node, treeType);
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

void assertPathsFromNodeHaveSameBlackHeight(RedBlackNode *tree, U64 nodes,
                                            RedBlackTreeType treeType,
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
                    appendRedBlackTreeWithBadNode(tree, node, treeType);

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
