#ifndef SHARED_TREES_RED_BLACK_MEMORY_MANAGER_H
#define SHARED_TREES_RED_BLACK_MEMORY_MANAGER_H

#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black-common.h"
#include "shared/types/numeric.h"

typedef struct RedBlackNodeMM RedBlackNodeMM;
struct RedBlackNodeMM {
    RedBlackNodeMM *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
    RedBlackColor color;
    Memory memory;
    U64 mostBytesInSubtree;
};

typedef ARRAY(RedBlackNodeMM *) RedBlackNodeMMPtr_a;

void insertRedBlackNodeMM(RedBlackNodeMM **tree, RedBlackNodeMM *createdNode);
RedBlackNodeMM *deleteAtLeastRedBlackNodeMM(RedBlackNodeMM **tree, U64 bytes);

#endif
