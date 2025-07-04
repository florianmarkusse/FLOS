#ifndef SHARED_TREES_RED_BLACK_COMMON_H
#define SHARED_TREES_RED_BLACK_COMMON_H

#include "shared/types/array.h"
#include "shared/types/numeric.h"
typedef enum { RB_TREE_BLACK = 0, RB_TREE_RED = 1 } RedBlackColor;
typedef enum { RB_TREE_LEFT = 0, RB_TREE_RIGHT = 1 } RedBlackDirection;

static constexpr auto RB_TREE_CHILD_COUNT = 2;
static constexpr auto RB_TREE_MAX_HEIGHT = 128;

RedBlackDirection calculateDirection(U64 value, U64 toCompare);

typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    RedBlackNode *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
};

typedef struct {
    RedBlackNode *node;
    RedBlackDirection direction;
} CommonVisitedNode;

void rotateAround(RedBlackNode *rotationParent, RedBlackNode *rotationNode,
                  RedBlackNode *rotationChild,
                  RedBlackDirection rotationDirection,
                  RedBlackDirection parentToChildDirection);

U64 findAdjacentInSteps(RedBlackNode *node, CommonVisitedNode *visitedNodes,
                        RedBlackDirection direction);

#endif
