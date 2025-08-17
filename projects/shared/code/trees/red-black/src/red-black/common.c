#include "shared/trees/red-black/common.h"

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

U32 findAdjacentInSteps(RedBlackNode *node, CommonVisitedNode *visitedNodes,
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
