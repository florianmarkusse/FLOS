#ifndef SHARED_TREES_RED_BLACK_COMMON_H
#define SHARED_TREES_RED_BLACK_COMMON_H

#include "shared/types/array.h"
#include "shared/types/numeric.h"
typedef enum { RB_TREE_BLACK = 0, RB_TREE_RED = 1 } RedBlackColor;
typedef enum { RB_TREE_LEFT = 0, RB_TREE_RIGHT = 1 } RedBlackDirection;

static constexpr auto RB_TREE_CHILD_COUNT = 2;
static constexpr auto RB_TREE_MAX_HEIGHT = 128;

RedBlackDirection redBlackCalculateDirection(U64 value, U64 toCompare);

// TODO: Having color here as the second argument causes some likely extra
// padding in structs that is unneeded. However, for the polymorphism to work,
// we need it for now. Later, can be looked into how to make this work with the
// polymorphism and the freedom to pack the struct as efficiently as possible
typedef struct RedBlackNode RedBlackNode;
struct RedBlackNode {
    RedBlackNode *
        children[RB_TREE_CHILD_COUNT]; // NOTE: Keep this as the first elements.
                                       // This is used in the insert so that
                                       // children->[0] and a RedBlackNode* are
                                       // the same location for doing inserts.
    RedBlackColor color;               // NOTE: Keep this as the second element
};

typedef struct {
    RedBlackNode *node;
    RedBlackDirection direction;
} CommonNodeVisited;

typedef void (*RotationUpdater)(void *rotationNode, void *rotationChild);

[[nodiscard]] U32
redBlackRebalanceInsert(RedBlackDirection direction,
                        CommonNodeVisited visitedNodes[RB_TREE_MAX_HEIGHT],
                        U32 len, RotationUpdater rotationUpdater);
[[nodiscard]] U32
redBlackRebalanceDelete(RedBlackDirection direction,
                        CommonNodeVisited visitedNodes[RB_TREE_MAX_HEIGHT],
                        U32 len, RotationUpdater rotationUpdater);

void redBlackRotate(RedBlackNode *rotationParent, RedBlackNode *rotationNode,
                    RedBlackNode *rotationChild,
                    RedBlackDirection rotationDirection,
                    RedBlackDirection parentToChildDirection);

[[nodiscard]] U32 redBlackfindAdjacentInSteps(RedBlackNode *node,
                                              CommonNodeVisited *visitedNodes,
                                              RedBlackDirection direction);

void redBlackChildrenPreOrderAdd(RedBlackNode *current, RedBlackNode **buffer,
                                 U32 *currentLen);

#define RB_TREE_TRAVERSAL_PRE_ORDER(node, len, buffer)                         \
    for (; (len) > 0 && ((node) = (buffer)[(len) - 1]);                        \
         (len) -= 1,                                                           \
         redBlackChildrenPreOrderAdd((RedBlackNode *)(node),                   \
                                     (RedBlackNode **)(buffer), &(len)))

#endif
