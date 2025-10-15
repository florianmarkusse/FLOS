#include "shared/trees/red-black/tests/assert.h"
#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"

static RedBlackColor getColor(RedBlackNode *node, RedBlackTreeType treeType) {
    switch (treeType) {
    case RED_BLACK_BASIC: {
        RedBlackNodeBasic *basicNode = (RedBlackNodeBasic *)node;
        return basicNode->color;
    }
    }
}

static void printTreeIndented(RedBlackNode *node, int depth, String prefix,
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

U32 nodeCount(RedBlackNode *tree, RedBlackTreeType treeType) {
    RedBlackNode *buffer[RB_TREE_MAX_HEIGHT];

    U32 result = 0;

    buffer[0] = tree;
    U32 len = 1;
    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;
        result++;

        for (RedBlackDirection dir = 0; dir < RB_TREE_CHILD_COUNT; dir++) {
            if (node->children[dir]) {
                if (len > RB_TREE_MAX_HEIGHT) {
                    TEST_FAILURE {
                        INFO(STRING(
                            "Tree has too many nodes to assert correctness!"));
                        appendRedBlackTreeWithBadNode(tree, tree, treeType);
                    }
                }
                buffer[len] = node->children[dir];
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

void assertNoRedNodeHasRedChild(RedBlackNode *tree, U32 nodes,
                                RedBlackTreeType treeType, Arena scratch) {
    RedBlackNode **buffer = NEW(&scratch, RedBlackNode *, .count = nodes);
    U32 len = 0;

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

        for (RedBlackDirection dir = 0; dir < RB_TREE_CHILD_COUNT; dir++) {
            if (node->children[dir]) {
                buffer[len] = node->children[dir];
                len++;
            }
        }
    }
}

static void collectBlackHeightsForEachPath(RedBlackNode *node,
                                           U32_a *blackHeights, U32 current,
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

void assertPathsFromNodeHaveSameBlackHeight(RedBlackNode *tree, U32 nodes,
                                            RedBlackTreeType treeType,
                                            Arena scratch) {
    RedBlackNode **buffer = NEW(&scratch, RedBlackNode *, .count = nodes);
    U32 len = 0;

    buffer[len] = tree;
    len++;

    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;

        // Stops at leaf nodes, so nodes + 1 is max lenght
        U32_a blackHeights = {.buf = NEW(&scratch, U32, .count = nodes + 1),
                              .len = 0};

        collectBlackHeightsForEachPath(node, &blackHeights, 0, treeType);

        U32 first = blackHeights.buf[0];
        for (typeof(blackHeights.len) i = 1; i < blackHeights.len; i++) {
            if (blackHeights.buf[i] != first) {
                TEST_FAILURE {
                    INFO(STRING("Found differing black heights!\n"));
                    appendRedBlackTreeWithBadNode(tree, node, treeType);

                    INFO(STRING("Black heights calculated (leftmost leaf node "
                                "to rightmost leaf node):"));
                    for (typeof(blackHeights.len) j = 0; j < blackHeights.len;
                         j++) {
                        INFO(blackHeights.buf[j]);
                        INFO(STRING(" "));
                    }
                    INFO(STRING("\n"));
                }
            }
        }

        for (RedBlackDirection dir = 0; dir < RB_TREE_CHILD_COUNT; dir++) {
            if (node->children[dir]) {
                buffer[len] = node->children[dir];
                len++;
            }
        }
    }
}
