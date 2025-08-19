#include "shared/trees/red-black/tests/assert.h"
#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/trees/red-black/memory-manager.h"

static RedBlackColor getColor(RedBlackNode *node, RedBlackTreeType treeType) {
    switch (treeType) {
    case RED_BLACK_BASIC: {
        RedBlackNodeBasic *basicNode = (RedBlackNodeBasic *)node;
        return basicNode->color;
    }
    case RED_BLACK_MEMORY_MANAGER: {
        MMNode *memoryManagerNode = (MMNode *)node;
        return memoryManagerNode->color;
    }
    }
}

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
    INFO(getColor(node, treeType) == RB_TREE_RED ? STRING("RED")
                                                 : STRING("BLACK"));

    switch (treeType) {
    case RED_BLACK_BASIC: {
        RedBlackNodeBasic *basicNode = (RedBlackNodeBasic *)node;
        INFO(STRING(" Value: "));
        INFO(basicNode->value);
        break;
    }
    case RED_BLACK_MEMORY_MANAGER: {
        MMNode *memoryManagerNode = (MMNode *)node;
        INFO(STRING(" Start: "));
        INFO(memoryManagerNode->memory.start);
        INFO(STRING(" Bytes: "));
        INFO(memoryManagerNode->memory.bytes);
        INFO(STRING(" Most bytes in subtree: "));
        INFO(memoryManagerNode->mostBytesInSubtree);
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
    INFO(STRING("Red-Black Tree Structure:"), .flags = NEWLINE);
    printTreeIndented(root, 0, STRING("Root---"), badNode, treeType);
}

U64 nodeCount(RedBlackNode *tree, RedBlackTreeType treeType) {
    RedBlackNode *buffer[RB_TREE_MAX_HEIGHT];

    U64 result = 0;

    buffer[0] = tree;
    U64 len = 1;
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
                                 RedBlackDirection direction,
                                 RedBlackTreeType treeType) {
    if (node->children[direction] &&
        getColor(node->children[direction], treeType) == RB_TREE_RED) {
        return true;
    }

    return false;
}

void assertNoRedNodeHasRedChild(RedBlackNode *tree, U64 nodes,
                                RedBlackTreeType treeType, Arena scratch) {
    RedBlackNode **buffer = NEW(&scratch, RedBlackNode *, .count = nodes);
    U64 len = 0;

    buffer[len] = tree;
    len++;
    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;

        if (getColor(node, treeType) == RB_TREE_RED) {
            if (redParentHasRedChild(node, RB_TREE_LEFT, treeType) ||
                redParentHasRedChild(node, RB_TREE_RIGHT, treeType)) {
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
                                           U64_a *blackHeights, U64 current,
                                           RedBlackTreeType treeType) {
    if (!node) {
        blackHeights->buf[blackHeights->len] = current + 1;
        blackHeights->len++;
    } else {
        if (getColor(node, treeType) == RB_TREE_BLACK) {
            current++;
        }

        collectBlackHeightsForEachPath(node->children[RB_TREE_LEFT],
                                       blackHeights, current, treeType);
        collectBlackHeightsForEachPath(node->children[RB_TREE_RIGHT],
                                       blackHeights, current, treeType);
    }
}

void assertPathsFromNodeHaveSameBlackHeight(RedBlackNode *tree, U64 nodes,
                                            RedBlackTreeType treeType,
                                            Arena scratch) {
    RedBlackNode **buffer = NEW(&scratch, RedBlackNode *, .count = nodes);
    U64 len = 0;

    buffer[len] = tree;
    len++;

    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;

        U64 *_blackHeightsBuffer = NEW(&scratch, U64, .count = nodes);
        U64_a blackHeights = {.buf = _blackHeightsBuffer, .len = 0};

        collectBlackHeightsForEachPath(node, &blackHeights, 0, treeType);

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
