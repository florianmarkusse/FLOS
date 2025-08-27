#include "shared/trees/red-black/tests/assert.h"
#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/trees/red-black/virtual-mapping-manager.h"

static void printTreeIndented(NodeLocation *nodeLocation, U32 node, int depth,
                              String prefix, U32 badNode,
                              RedBlackTreeType treeType) {
    if (!node) {
        return;
    }

    printTreeIndented(nodeLocation,
                      getNode(nodeLocation, node)->children[RB_TREE_RIGHT],
                      depth + 1, STRING("R---"), badNode, treeType);

    for (int i = 0; i < depth; i++) {
        INFO(STRING("    "));
    }
    INFO(prefix);
    INFO(STRING(", Color: "));
    INFO(getColor(nodeLocation, node) == RB_TREE_RED ? STRING("RED")
                                                     : STRING("BLACK"));

    RedBlackNode *treeNode = getNode(nodeLocation, node);
    switch (treeType) {
    case RED_BLACK_VIRTUAL_MEMORY_MAPPER: {
        VMMNode *vmmNode = (VMMNode *)treeNode;
        INFO(STRING(" Value: "));
        INFO(vmmNode->data.memory.start);
        break;
    }
    case RED_BLACK_MEMORY_MANAGER: {
        MMNode *memoryManagerNode = (MMNode *)treeNode;
        INFO(STRING(" Start: "));
        INFO(memoryManagerNode->data.memory.start);
        INFO(STRING(" Bytes: "));
        INFO(memoryManagerNode->data.memory.bytes);
        INFO(STRING(" Most bytes in subtree: "));
        INFO(memoryManagerNode->data.mostBytesInSubtree);
        break;
    }
    }

    if (node == badNode) {
        appendColor(COLOR_RED, STDOUT);
        INFO(STRING("    !!!!    !!!!"));
        appendColorReset(STDOUT);
    }

    INFO(STRING("\n"));

    printTreeIndented(nodeLocation, treeNode->children[RB_TREE_LEFT], depth + 1,
                      STRING("L---"), badNode, treeType);
}

void appendRedBlackTreeWithBadNode(NodeLocation *nodeLocation, U32 tree,
                                   U32 badNode, RedBlackTreeType treeType) {
    INFO(STRING("Red-Black Tree Structure:"), .flags = NEWLINE);
    printTreeIndented(nodeLocation, tree, 0, STRING("Root---"), badNode,
                      treeType);
}

U32 nodeCount(NodeLocation *nodeLocation, U32 tree, RedBlackTreeType treeType) {
    U32 buffer[RB_TREE_MAX_HEIGHT];

    U32 result = 0;

    buffer[0] = tree;
    U32 len = 1;
    while (len > 0) {
        RedBlackNode *node = getNode(nodeLocation, buffer[len - 1]);
        len--;
        result++;

        for (U32 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            if (node->children[i]) {
                if (len > RB_TREE_MAX_HEIGHT) {
                    TEST_FAILURE {
                        INFO(STRING(
                            "Tree has too many nodes to assert correctness!"));
                        appendRedBlackTreeWithBadNode(nodeLocation, tree, tree,
                                                      treeType);
                    }
                }
                buffer[len] = node->children[i];
                len++;
            }
        }
    }

    return result;
}

static bool redParentHasRedChild(NodeLocation *nodeLocation, U32 node,
                                 RedBlackDirection direction) {
    RedBlackNode *treeNode = getNode(nodeLocation, node);
    if (treeNode->children[direction] &&
        getColor(nodeLocation, treeNode->children[direction]) == RB_TREE_RED) {
        return true;
    }

    return false;
}

void assertNoRedNodeHasRedChild(NodeLocation *nodeLocation, U32 tree, U32 nodes,
                                RedBlackTreeType treeType, Arena scratch) {
    U32 *buffer = NEW(&scratch, U32, .count = nodes);
    U32 len = 0;

    buffer[len] = tree;
    len++;
    while (len > 0) {
        U32 node = buffer[len - 1];
        len--;

        if (getColor(nodeLocation, node) == RB_TREE_RED) {
            if (redParentHasRedChild(nodeLocation, node, RB_TREE_LEFT) ||
                redParentHasRedChild(nodeLocation, node, RB_TREE_RIGHT)) {
                TEST_FAILURE {
                    INFO(STRING("Red node has a red child!\n"));
                    appendRedBlackTreeWithBadNode(nodeLocation, tree, node,
                                                  treeType);
                }
            }
        }

        for (U32 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            RedBlackNode *treeNode = getNode(nodeLocation, node);
            if (treeNode->children[i]) {
                buffer[len] = treeNode->children[i];
                len++;
            }
        }
    }
}

static void collectBlackHeightsForEachPath(NodeLocation *nodeLocation, U32 node,
                                           U32_a *blackHeights, U32 current) {
    if (!node) {
        blackHeights->buf[blackHeights->len] = current + 1;
        blackHeights->len++;
    } else {
        RedBlackNode *treeNode = getNode(nodeLocation, node);
        if (getColorWithPointer(treeNode) == RB_TREE_BLACK) {
            current++;
        }

        collectBlackHeightsForEachPath(nodeLocation,
                                       treeNode->children[RB_TREE_LEFT],
                                       blackHeights, current);
        collectBlackHeightsForEachPath(nodeLocation,
                                       treeNode->children[RB_TREE_RIGHT],
                                       blackHeights, current);
    }
}

void assertPathsFromNodeHaveSameBlackHeight(NodeLocation *nodeLocation,
                                            U32 tree, U32 nodes,
                                            RedBlackTreeType treeType,
                                            Arena scratch) {
    U32 *buffer = NEW(&scratch, U32, .count = nodes);
    U32 len = 0;

    buffer[len] = tree;
    len++;

    while (len > 0) {
        U32 node = buffer[len - 1];
        len--;

        // Stops at leaf nodes, so nodes + 1 is max lenght
        U32_a blackHeights = {.buf = NEW(&scratch, U32, .count = nodes + 1),
                              .len = 0};

        collectBlackHeightsForEachPath(nodeLocation, node, &blackHeights, 0);

        U32 first = blackHeights.buf[0];
        for (U32 i = 1; i < blackHeights.len; i++) {
            if (blackHeights.buf[i] != first) {
                TEST_FAILURE {
                    INFO(STRING("Found differing black heights!\n"));
                    appendRedBlackTreeWithBadNode(nodeLocation, tree, node,
                                                  treeType);

                    INFO(STRING("Black heights calculated (leftmost leaf node "
                                "to rightmost leaf node):"));
                    for (U32 j = 0; j < blackHeights.len; j++) {
                        INFO(blackHeights.buf[j]);
                        INFO(STRING(" "));
                    }
                    INFO(STRING("\n"));
                }
            }
        }

        RedBlackNode *treeNode = getNode(nodeLocation, node);
        for (U32 i = 0; i < RB_TREE_CHILD_COUNT; i++) {
            if (treeNode->children[i]) {
                buffer[len] = treeNode->children[i];
                len++;
            }
        }
    }
}
