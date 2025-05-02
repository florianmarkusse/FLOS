#ifndef SHARED_TREES_RED_BLACK_TESTS_ASSERT_H
#define SHARED_TREES_RED_BLACK_TESTS_ASSERT_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black-basic.h"
#include "shared/types/numeric.h"

static constexpr auto MAX_NODES_IN_TREE = 1024;

typedef enum { RED_BLACK_BASIC, RED_BLACK_MEMORY_MANAGER } RedBlackTreeType;

typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    RedBlackNode *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
    RedBlackColor color;
};

void appendRedBlackTreeWithBadNode(RedBlackNode *root, RedBlackNode *badNode,
                                   RedBlackTreeType treeType);

U64 nodeCount(RedBlackNode *tree, RedBlackTreeType treeType);
void assertNoRedNodeHasRedChild(RedBlackNode *tree, U64 nodes,
                                RedBlackTreeType treeType, Arena scratch);
void assertPathsFromNodeHaveSameBlackHeight(RedBlackNode *tree, U64 nodes,
                                            RedBlackTreeType treeType,
                                            Arena scratch);

#endif
