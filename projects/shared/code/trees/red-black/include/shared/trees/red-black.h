#ifndef SHARED_TREES_RED_BLACK_H
#define SHARED_TREES_RED_BLACK_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/types.h"

typedef enum { BLACK = 0, RED = 1 } RedBlackColor;
typedef enum { LEFT_CHILD = 0, RIGHT_CHILD = 1 } RedBlackDirection;

static constexpr auto CHILD_COUNT = 2;

typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    RedBlackNode
        *children[CHILD_COUNT]; // NOTE: Keep this as the first elements. This
                                // is used in the insert so that children->[0]
                                // and a RedBlackNode* are the same location for
                                // doing inserts.
    RedBlackColor color;
    U64 value;
};

void insertRedBlackNode(RedBlackNode **tree, RedBlackNode *createdNode);
RedBlackNode *deleteRedBlackNode(RedBlackNode **tree, U64 value);
RedBlackNode *findRedBlackNodeLeastBiggestValue(RedBlackNode *tree, U64 value);

#endif
