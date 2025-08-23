#ifndef SHARED_TREES_RED_BLACK_BASIC_H
#define SHARED_TREES_RED_BLACK_BASIC_H

#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/common.h"

typedef struct RedBlackNodeBasic RedBlackNodeBasic;
struct RedBlackNodeBasic {
    RedBlackNodeBasic *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
                                       // And as polymorphism for some common
                                       // operations.
    U64 value;
    RedBlackColor color;
};

typedef struct {
    RedBlackNodeBasic *node;
    RedBlackDirection direction;
} VisitedNode;

typedef void (*RotationUpdater)(void *rotationNode, void *rotationChild);

typedef ARRAY(RedBlackNodeBasic *) RedBlackNodeBasicPtr_a;

typedef MAX_LENGTH_ARRAY(RedBlackNodeBasic) RedBlackNodeBasic_max_a;
typedef MAX_LENGTH_ARRAY(RedBlackNodeBasic *) RedBlackNodeBasicPtr_max_a;

U32 rebalanceInsert(RedBlackDirection direction,
                    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                    RotationUpdater rotationUpdater);

void insertRedBlackNodeBasic(RedBlackNodeBasic **tree,
                             RedBlackNodeBasic *createdNode);
RedBlackNodeBasic *deleteRedBlackNodeBasic(RedBlackNodeBasic **tree, U64 value);
RedBlackNodeBasic *deleteAtLeastRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                                  U64 value);
RedBlackNodeBasic *findGreatestBelowOrEqual(RedBlackNodeBasic **tree,
                                            U64 value);

#endif
