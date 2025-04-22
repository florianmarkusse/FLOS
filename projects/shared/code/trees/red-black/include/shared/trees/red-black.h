#ifndef SHARED_TREES_RED_BLACK_H
#define SHARED_TREES_RED_BLACK_H

#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"

typedef enum { RB_TREE_BLACK = 0, RB_TREE_RED = 1 } RedBlackColor;
typedef enum { RB_TREE_LEFT = 0, RB_TREE_RIGHT = 1 } RedBlackDirection;

static constexpr auto RB_TREE_CHILD_COUNT = 2;

typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    RedBlackNode *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
    RedBlackColor color;
    Memory memory;
};

typedef ARRAY(RedBlackNode *) RedBlackNodePtr_a;

void insertRedBlackNode(RedBlackNode **tree, RedBlackNode *createdNode);
RedBlackNode *deleteRedBlackNode(RedBlackNode **tree, U64 value);
RedBlackNode *deleteAtLeastRedBlackNode(RedBlackNode **tree, U64 value);

#endif
