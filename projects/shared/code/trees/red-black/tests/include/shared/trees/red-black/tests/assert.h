#ifndef SHARED_TREES_RED_BLACK_TESTS_ASSERT_H
#define SHARED_TREES_RED_BLACK_TESTS_ASSERT_H

#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/basic.h"
#include "shared/trees/red-black/common.h"
#include "shared/types/numeric.h"

static constexpr auto MAX_NODES_IN_TREE = 1024;

typedef enum {
    RED_BLACK_VIRTUAL_MEMORY_MAPPER,
    RED_BLACK_MEMORY_MANAGER
} RedBlackTreeType;

void appendRedBlackTreeWithBadNode(NodeLocation *nodeLocation, U32 tree,
                                   U32 badNode, RedBlackTreeType treeType);

U32 nodeCount(NodeLocation *nodeLocation, U32 tree, RedBlackTreeType treeType);
void assertNoRedNodeHasRedChild(NodeLocation *nodeLocation, U32 tree, U32 nodes,
                                RedBlackTreeType treeType, Arena scratch);
void assertPathsFromNodeHaveSameBlackHeight(NodeLocation *nodeLocation,
                                            U32 tree, U32 nodes,
                                            RedBlackTreeType treeType,
                                            Arena scratch);

#endif
