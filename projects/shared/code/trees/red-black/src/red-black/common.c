#include "shared/trees/red-black/common.h"

U32 rebalanceInsert(RedBlackDirection direction,
                    CommonNodeVisited visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                    RotationUpdater rotationUpdater) {
    RedBlackNode *grandParent = visitedNodes[len - 3].node;
    RedBlackNode *parent = visitedNodes[len - 2].node;
    RedBlackNode *node = visitedNodes[len - 1].node;

    RedBlackNode *uncle = grandParent->children[!direction];
    if (uncle && uncle->color == RB_TREE_RED) {
        uncle->color = RB_TREE_BLACK;
        parent->color = RB_TREE_BLACK;
        grandParent->color = RB_TREE_RED;

        return len - 2;
    }

    //      x             x
    //     /             /
    //    y       ==>   z
    //     \           /
    //      z         y
    if (visitedNodes[len - 2].direction == !direction) {
        rotateAround((RedBlackNode *)grandParent, (RedBlackNode *)parent,
                     (RedBlackNode *)node, direction, direction);
        if (rotationUpdater) {
            rotationUpdater(parent, node);
        }

        node = node->children[direction];
        parent = grandParent->children[direction];
    }

    //      x           y
    //     /           / \
    //    y      ==>  z   x
    //   /
    //  z
    parent->color = RB_TREE_BLACK;
    grandParent->color = RB_TREE_RED;

    rotateAround((RedBlackNode *)visitedNodes[len - 4].node,
                 (RedBlackNode *)grandParent, (RedBlackNode *)parent,
                 !direction, visitedNodes[len - 4].direction);
    if (rotationUpdater) {
        rotationUpdater(grandParent, parent);
    }

    return 0;
}

// We have 2 subtrees hanging from visitedNodes - 1, the subtree of direction
// and the subtree of !direction (other direction). the subtree of direction has
// 1 less black height than the other subtree. The potential tree above these
// subtrees is now also missing a black node. Fixing the deficiency by removing
// a black node from the other direction subtree means that we stil need to
// address that problem. On the other hand, coloring a node black in the
// direction subtree immediately solves the deficiency in the whole tree.
U32 rebalanceDelete(RedBlackDirection direction,
                    CommonNodeVisited visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                    RotationUpdater rotationUpdater) {
    RedBlackNode *node = visitedNodes[len - 1].node;
    RedBlackNode *childOtherDirection = node->children[!direction];
    // Ensure the other child is colored black, we "push" the problem a level
    // down in the process.
    //                       x(B)              y(B)
    //                        \               / \
    // LEFT_DIRECTION          y(R)   ==>   x(R)a(B)
    //                        / \            \
    //                      z(B)a(B)        z(B)
    //
    //                        x(B)            y(B)
    //                        /               / \
    // RIGHT_DIRECTION     y(R)       ==>   a(B)x(R)
    //                     / \                  /
    //                   a(B)z(B)             z(B)
    if (childOtherDirection->color == RB_TREE_RED) {
        childOtherDirection->color = RB_TREE_BLACK;
        node->color = RB_TREE_RED;

        rotateAround((RedBlackNode *)visitedNodes[len - 2].node,
                     (RedBlackNode *)node, (RedBlackNode *)childOtherDirection,
                     direction, visitedNodes[len - 2].direction);
        if (rotationUpdater) {
            rotationUpdater(node, childOtherDirection);
        }

        visitedNodes[len - 1].node = childOtherDirection;
        visitedNodes[len].node = node;
        visitedNodes[len].direction = direction;
        len++;

        childOtherDirection = node->children[!direction];
    }

    RedBlackNode *innerChildOtherDirection =
        childOtherDirection->children[direction];
    RedBlackNode *outerChildOtherDirection =
        childOtherDirection->children[!direction];
    // Bubble up the problem by 1 level.
    if (((!innerChildOtherDirection) ||
         innerChildOtherDirection->color == RB_TREE_BLACK) &&
        ((!outerChildOtherDirection) ||
         outerChildOtherDirection->color == RB_TREE_BLACK)) {
        childOtherDirection->color = RB_TREE_RED;

        return len - 1;
    }

    //                      x                 x
    //                       \                 \
    // LEFT_DIRECTION         y(B)   ===>     z(B)
    //                       /                   \
    //                     z(R)                  y(R)
    //
    //                      x                     x
    //                     /                     /
    // RIGHT_DIRECTION   y(B)        ===>      z(B)
    //                     \                   /
    //                     z(R)             y(R)
    if ((!outerChildOtherDirection) ||
        outerChildOtherDirection->color == RB_TREE_BLACK) {
        childOtherDirection->color = RB_TREE_RED;
        innerChildOtherDirection->color = RB_TREE_BLACK;

        rotateAround((RedBlackNode *)node, (RedBlackNode *)childOtherDirection,
                     (RedBlackNode *)innerChildOtherDirection, !direction,
                     !direction);
        if (rotationUpdater) {
            rotationUpdater(childOtherDirection, innerChildOtherDirection);
        }

        RedBlackNode *temp = childOtherDirection;
        childOtherDirection = innerChildOtherDirection;
        outerChildOtherDirection = temp;
    }

    //                         x                          y
    //                        / \                        / \
    // LEFT_DIRECTION       a(B)y(B)        ===>       x(B)z(B)
    //                            \                    /
    //                           z(R)                a(B)
    //
    //                         x                          y
    //                        / \                        / \
    // RIGHT_DIRECTION      y(B)a(B)        ===>       z(B)x(B)
    //                      /                               \
    //                    z(R)                              a(B)
    childOtherDirection->color = node->color;
    node->color = RB_TREE_BLACK;
    outerChildOtherDirection->color = RB_TREE_BLACK;

    rotateAround((RedBlackNode *)visitedNodes[len - 2].node,
                 (RedBlackNode *)node, (RedBlackNode *)childOtherDirection,
                 direction, visitedNodes[len - 2].direction);
    if (rotationUpdater) {
        rotationUpdater(node, childOtherDirection);
    }

    return 0;
}

RedBlackDirection calculateDirection(U64 value, U64 toCompare) {
    if (value >= toCompare) {
        return RB_TREE_RIGHT;
    }
    return RB_TREE_LEFT;
}

void rotateAround(RedBlackNode *rotationParent, RedBlackNode *rotationNode,
                  RedBlackNode *rotationChild,
                  RedBlackDirection rotationDirection,
                  RedBlackDirection parentToChildDirection) {
    rotationNode->children[!rotationDirection] =
        rotationChild->children[rotationDirection];
    rotationChild->children[rotationDirection] = rotationNode;
    rotationParent->children[parentToChildDirection] = rotationChild;
}

U32 findAdjacentInSteps(RedBlackNode *node, CommonNodeVisited *visitedNodes,
                        RedBlackDirection direction) {
    if (!node->children[direction]) {
        return 0;
    }

    U32 traversals = 0;

    visitedNodes[traversals].node = node;
    visitedNodes[traversals].direction = direction;
    node = node->children[direction];
    traversals++;

    while (true) {
        RedBlackNode *next = node->children[!direction];
        if (!next) {
            break;
        }

        visitedNodes[traversals].node = node;
        visitedNodes[traversals].direction = !direction;
        traversals++;

        node = next;
    }

    return traversals;
}
