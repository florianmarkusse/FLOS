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

typedef ARRAY(RedBlackNodeBasic *) RedBlackNodeBasicPtr_a;

void insertRedBlackNodeBasic(RedBlackNodeBasic **tree,
                             RedBlackNodeBasic *createdNode);
RedBlackNodeBasic *deleteRedBlackNodeBasic(RedBlackNodeBasic **tree, U64 value);
RedBlackNodeBasic *deleteAtLeastRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                                  U64 value);

#endif
