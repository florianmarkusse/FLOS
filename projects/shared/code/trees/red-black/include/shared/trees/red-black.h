#ifndef SHARED_TREES_RED_BLACK_H
#define SHARED_TREES_RED_BLACK_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/types.h"

typedef enum { BLACK = 0, RED = 1 } RedBlackColor;

static constexpr auto LEFT_CHILD = 0;
static constexpr auto RIGHT_CHILD = 1;

typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    U64 value;
    RedBlackColor color;
    RedBlackNode *children[2];
};

void insertRedBlackNode(RedBlackNode **node, U64 value, Arena *perm);
void deleteRedBlackNode(RedBlackNode *tree, RedBlackNode *node);
RedBlackNode *findRedBlackNodeLeastBiggestValue(RedBlackNode *tree, U64 value);

#endif
