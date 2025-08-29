#include "shared/trees/red-black/common.h"

// Just need a consistent index. Using 0 or 1 as index value works.
static constexpr auto RED_BLACK_COLOR_INDEX = 0;
static constexpr auto RED_BLACK_COLOR_SHIFT = 31;
static constexpr auto RED_BLACK_COLOR_MASK = (1ULL << RED_BLACK_COLOR_SHIFT);

U32 getIndex(NodeLocation *nodeLocation, void *node) {
    U64 difference = (U64)(((U8 *)node) - (nodeLocation->base));
    return (U32)(difference / nodeLocation->elementSizeBytes);
}

RedBlackNode *getNode(NodeLocation *nodeLocation, U32 index) {
    return (RedBlackNode *)(nodeLocation->base +
                            (index * nodeLocation->elementSizeBytes));
}

void setColorWithPointer(RedBlackNode *node, RedBlackColor color) {
    if (color == RB_TREE_BLACK) {
        node->metaData[RED_BLACK_COLOR_INDEX] &= (~RED_BLACK_COLOR_MASK);
    } else {
        node->metaData[RED_BLACK_COLOR_INDEX] |= RED_BLACK_COLOR_MASK;
    }
}

void setColor(NodeLocation *nodeLocation, U32 index, RedBlackColor color) {
    setColorWithPointer(getNode(nodeLocation, index), color);
}

RedBlackColor getColorWithPointer(RedBlackNode *node) {
    return (RedBlackColor)(node->metaData[RED_BLACK_COLOR_INDEX] >>
                           RED_BLACK_COLOR_SHIFT);
}

RedBlackColor getColor(NodeLocation *nodeLocation, U32 index) {
    return getColorWithPointer(getNode(nodeLocation, index));
}

void childNodePointerSet(RedBlackNode *parentNode, RedBlackDirection direction,
                         U32 childIndex) {
    parentNode->metaData[direction] =
        (parentNode->metaData[direction] & RED_BLACK_COLOR_MASK) | childIndex;
}

void childNodeIndexSet(NodeLocation *nodeLocation, U32 parent,
                       RedBlackDirection direction, U32 childIndex) {
    RedBlackNode *parentNode = getNode(nodeLocation, parent);
    childNodePointerSet(parentNode, direction, childIndex);
}

U32 childNodePointerGet(RedBlackNode *parentNode, RedBlackDirection direction) {
    return parentNode->metaData[direction] & (~RED_BLACK_COLOR_MASK);
}

U32 childNodeIndexGet(NodeLocation *nodeLocation, U32 parent,
                      RedBlackDirection direction) {
    RedBlackNode *parentNode = getNode(nodeLocation, parent);
    return childNodePointerGet(parentNode, direction);
}

U32 rebalanceInsert(NodeLocation *nodeLocation,
                    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                    U32 *tree, RedBlackDirection direction,
                    RotationUpdater rotationUpdater) {
    U32 grandParent = visitedNodes[len - 3].index;
    U32 parent = visitedNodes[len - 2].index;
    U32 node = visitedNodes[len - 1].index;

    U32 uncle = childNodeIndexGet(nodeLocation, grandParent, !direction);
    if (uncle && getColor(nodeLocation, uncle) == RB_TREE_RED) {
        setColor(nodeLocation, uncle, RB_TREE_BLACK);
        setColor(nodeLocation, parent, RB_TREE_BLACK);
        setColor(nodeLocation, grandParent, RB_TREE_RED);

        return len - 2;
    }

    //      x             x
    //     /             /
    //    y       ==>   z
    //     \           /
    //      z         y
    if (visitedNodes[len - 2].direction == !direction) {
        rotateAround(nodeLocation, tree, grandParent, parent, node, direction,
                     direction);
        if (rotationUpdater) {
            rotationUpdater(nodeLocation, parent, node);
        }

        node = childNodeIndexGet(nodeLocation, node, direction);
        parent = childNodeIndexGet(nodeLocation, grandParent, direction);
    }

    //      x           y
    //     /           / \
    //    y      ==>  z   x
    //   /
    //  z
    setColor(nodeLocation, parent, RB_TREE_BLACK);
    setColor(nodeLocation, grandParent, RB_TREE_RED);

    rotateAround(nodeLocation, tree, visitedNodes[len - 4].index, grandParent,
                 parent, !direction, visitedNodes[len - 4].direction);
    if (rotationUpdater) {
        rotationUpdater(nodeLocation, grandParent, parent);
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
U32 rebalanceDelete(NodeLocation *nodeLocation,
                    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                    U32 *tree, RedBlackDirection direction,
                    RotationUpdater rotationUpdater) {
    U32 node = visitedNodes[len - 1].index;
    U32 childOtherDirection = childNodeIndexGet(nodeLocation, node, !direction);
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
    if (getColor(nodeLocation, childOtherDirection) == RB_TREE_RED) {
        setColor(nodeLocation, childOtherDirection, RB_TREE_BLACK);
        setColor(nodeLocation, node, RB_TREE_RED);

        rotateAround(nodeLocation, tree, visitedNodes[len - 2].index, node,
                     childOtherDirection, direction,
                     visitedNodes[len - 2].direction);
        if (rotationUpdater) {
            rotationUpdater(nodeLocation, node, childOtherDirection);
        }

        visitedNodes[len - 1].index = childOtherDirection;
        visitedNodes[len].index = node;
        visitedNodes[len].direction = direction;
        len++;

        childOtherDirection = childNodeIndexGet(nodeLocation, node, !direction);
    }

    U32 innerChildOtherDirection =
        childNodeIndexGet(nodeLocation, childOtherDirection, direction);
    U32 outerChildOtherDirection =
        childNodeIndexGet(nodeLocation, childOtherDirection, !direction);
    // Bubble up the problem by 1 level.
    if (((!innerChildOtherDirection) ||
         getColor(nodeLocation, innerChildOtherDirection) == RB_TREE_BLACK) &&
        ((!outerChildOtherDirection) ||
         getColor(nodeLocation, outerChildOtherDirection) == RB_TREE_BLACK)) {
        setColor(nodeLocation, childOtherDirection, RB_TREE_RED);

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
        getColor(nodeLocation, outerChildOtherDirection) == RB_TREE_BLACK) {
        setColor(nodeLocation, childOtherDirection, RB_TREE_RED);
        setColor(nodeLocation, innerChildOtherDirection, RB_TREE_BLACK);

        rotateAround(nodeLocation, tree, node, childOtherDirection,
                     innerChildOtherDirection, !direction, !direction);
        if (rotationUpdater) {
            rotationUpdater(nodeLocation, childOtherDirection,
                            innerChildOtherDirection);
        }

        U32 temp = childOtherDirection;
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
    setColor(nodeLocation, childOtherDirection, getColor(nodeLocation, node));
    setColor(nodeLocation, node, RB_TREE_BLACK);
    setColor(nodeLocation, outerChildOtherDirection, RB_TREE_BLACK);

    rotateAround(nodeLocation, tree, visitedNodes[len - 2].index, node,
                 childOtherDirection, direction,
                 visitedNodes[len - 2].direction);
    if (rotationUpdater) {
        rotationUpdater(nodeLocation, node, childOtherDirection);
    }

    return 0;
}

RedBlackDirection calculateDirection(U64 value, U64 toCompare) {
    if (value >= toCompare) {
        return RB_TREE_RIGHT;
    }
    return RB_TREE_LEFT;
}

void rotateAround(NodeLocation *nodeLocation, U32 *tree, U32 rotationParent,
                  U32 rotationNode, U32 rotationChild,
                  RedBlackDirection rotationDirection,
                  RedBlackDirection parentToChildDirection) {
    RedBlackNode *rotationChildPtr = getNode(nodeLocation, rotationChild);

    childNodeIndexSet(nodeLocation, rotationNode, !rotationDirection,
                      childNodePointerGet(rotationChildPtr, rotationDirection));
    childNodePointerSet(rotationChildPtr, rotationDirection, rotationNode);
    if (rotationParent) {
        if (!rotationParent) {
            *tree = rotationChild;
        } else {
            RedBlackNode *rotationParentPtr =
                getNode(nodeLocation, rotationParent);
            childNodePointerSet(rotationParentPtr, parentToChildDirection,
                                rotationChild);
        }
    } else {
        *tree = rotationChild;
    }
}

U32 findAdjacentInSteps(NodeLocation *nodeLocation, VisitedNode *visitedNodes,
                        U32 node, RedBlackDirection direction) {
    U32 directionChildIndex = childNodeIndexGet(nodeLocation, node, direction);
    if (!directionChildIndex) {
        return 0;
    }

    U32 traversals = 0;

    visitedNodes[traversals].index = node;
    visitedNodes[traversals].direction = direction;
    node = directionChildIndex;
    traversals++;

    while (true) {
        U32 next = childNodeIndexGet(nodeLocation, node, !direction);
        if (!next) {
            break;
        }

        visitedNodes[traversals].index = node;
        visitedNodes[traversals].direction = !direction;
        traversals++;

        node = next;
    }

    return traversals;
}
