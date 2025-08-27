#ifndef SHARED_TREES_RED_BLACK_MEMORY_MANAGER_H
#define SHARED_TREES_RED_BLACK_MEMORY_MANAGER_H

#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/common.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

// This is not just a red-black tree, but also an interval tree and merges
// contiguous nodes. So, inserts/deletes need some additional housekeeping to
// stay cnsistent.

typedef struct {
    Memory memory;
    U64 mostBytesInSubtree;
} MMData;

typedef struct MMNode MMNode;
struct MMNode {
    RedBlackNode header;
    MMData data;
};

typedef MAX_LENGTH_ARRAY(MMNode) MMNode_max_a;
typedef TREE_WITH_FREELIST(MMNode) MMTreeWithFreeList;

static constexpr auto RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT = 2;

typedef struct {
    U32 freed[RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT];
} InsertResult;

MMNode *getMMNode(NodeLocation *nodeLocation, U32 index);

// On inserting a node in this tree, there are 3 possibilities and 3
// different return values:
//  - a bridge merge with 2 other nodes: return 2 freed nodes
//  - a single merge with 1 other node: return 1 freed node
//  - no merges with other nodes: return 0 freed nodes
[[nodiscard]] InsertResult insertMMNode(MMTreeWithFreeList *treeWithFreeList,
                                        MMNode *createdNode);
[[nodiscard]] U32 deleteAtLeastMMNode(MMTreeWithFreeList *treeWithFreeList,
                                      U64 bytes);

#endif
