#ifndef SHARED_TREES_RED_BLACK_MEMORY_MANAGER_H
#define SHARED_TREES_RED_BLACK_MEMORY_MANAGER_H

#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/common.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

typedef struct RedBlackNodeMM RedBlackNodeMM;
struct RedBlackNodeMM {
    RedBlackNodeMM *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
    Memory memory;
    U64 mostBytesInSubtree;
    RedBlackColor color;
};

typedef MAX_LENGTH_ARRAY(RedBlackNodeMM) RedBlackNodeMM_max_a;
typedef PACKED_MAX_LENGTH_ARRAY(RedBlackNodeMM) PackedRedBlackNodeMM_max_a;
typedef MAX_LENGTH_ARRAY(RedBlackNodeMM *) RedBlackNodeMMPtr_max_a;
typedef PACKED_MAX_LENGTH_ARRAY(RedBlackNodeMM *) PackedRedBlackNodeMMPtr_max_a;

static constexpr auto RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT = 2;

typedef struct {
    RedBlackNodeMM *freed[RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT];
} InsertResult;

// On inserting a node in this tree, there are 3 possibilities and 3
// different return values:
//  - a bridge merge with 2 other nodes: return 2 freed nodes
//  - a single merge with 1 other node: return 1 freed node
//  - no merges with other nodes: return 0 freed nodes
[[nodiscard]] InsertResult insertRedBlackNodeMM(RedBlackNodeMM **tree,
                                                RedBlackNodeMM *createdNode);
[[nodiscard]] RedBlackNodeMM *deleteAtLeastRedBlackNodeMM(RedBlackNodeMM **tree,
                                                          U64 bytes);

#endif
