#ifndef SHARED_TREES_RED_BLACK_H
#define SHARED_TREES_RED_BLACK_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/types.h"

typedef enum { BLACK = 0, RED = 1 } RedBlackColor;
typedef enum { LEFT_CHILD = 0, RIGHT_CHILD = 1 } RedBlackDirection;

static constexpr auto CHILD_COUNT = 2;

typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    U64 value;
    RedBlackColor color;
    RedBlackNode *children[CHILD_COUNT];
};

void insertRedBlackNode(RedBlackNode **tree, RedBlackNode *createdNode);
void deleteRedBlackNode(RedBlackNode *tree, RedBlackNode *node);
RedBlackNode *findRedBlackNodeLeastBiggestValue(RedBlackNode *tree, U64 value);

#endif
