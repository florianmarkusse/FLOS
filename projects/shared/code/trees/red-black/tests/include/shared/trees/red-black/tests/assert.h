#ifndef SHARED_TREES_RED_BLACK_TESTS_ASSERT_H
#define SHARED_TREES_RED_BLACK_TESTS_ASSERT_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/basic.h"
#include "shared/trees/red-black/common.h"
#include "shared/types/numeric.h"

static constexpr auto MAX_NODES_IN_TREE = 16384;

typedef enum {
    RED_BLACK_VIRTUAL_MEMORY_MAPPER,
    RED_BLACK_MEMORY_MANAGER
} RedBlackTreeType;

void appendRedBlackTreeWithBadNode(TreeWithFreeList *treeWithFreeList,
                                   U32 badNode, RedBlackTreeType treeType);

U32 nodeCount(TreeWithFreeList *treeWithFreeList, RedBlackTreeType treeType);
void assertNoRedNodeHasRedChild(TreeWithFreeList *treeWithFreeList, U32 nodes,
                                RedBlackTreeType treeType, Arena scratch);
void assertPathsFromNodeHaveSameBlackHeight(TreeWithFreeList *treeWithFreeList,
                                            U32 nodes,
                                            RedBlackTreeType treeType,
                                            Arena scratch);

#endif
