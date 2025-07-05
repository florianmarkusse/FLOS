#include "shared/trees/red-black/basic.h"
#include "shared/maths.h"

typedef struct {
    RedBlackNodeBasic *node;
    RedBlackDirection direction;
} VisitedNode;

static U64 rebalanceInsert(RedBlackDirection direction,
                           VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                           U64 len) {
    RedBlackNodeBasic *grandParent = visitedNodes[len - 3].node;
    RedBlackNodeBasic *parent = visitedNodes[len - 2].node;
    RedBlackNodeBasic *node = visitedNodes[len - 1].node;

    RedBlackNodeBasic *uncle = grandParent->children[!direction];
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

    return 0;
}

RedBlackNodeBasic *findGreatestBelowOrEqual(RedBlackNodeBasic **tree,
                                            U64 value) {
    RedBlackNodeBasic *current = *tree;
    RedBlackNodeBasic *result = nullptr;

    while (current) {
        if (current->value == value) {
            return current;
        } else if (current->value < value) {
            result = current;
            current = current->children[RB_TREE_RIGHT];
        } else {
            current = current->children[RB_TREE_LEFT];
        }
    }
    return result;
}

void insertRedBlackNodeBasic(RedBlackNodeBasic **tree,
                             RedBlackNodeBasic *createdNode) {
    createdNode->children[RB_TREE_LEFT] = nullptr;
    createdNode->children[RB_TREE_RIGHT] = nullptr;

    if (!(*tree)) {
        createdNode->color = RB_TREE_BLACK;
        *tree = createdNode;
        return;
    }

    // Search
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    RedBlackNodeBasic *current = *tree;
    while (1) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction =
            calculateDirection(createdNode->value, current->value);
        len++;

        RedBlackNodeBasic *next =
            current->children[visitedNodes[len - 1].direction];
        if (!next) {
            break;
        }
        current = next;
    }

    // Insert
    createdNode->color = RB_TREE_RED;
    current->children[visitedNodes[len - 1].direction] = createdNode;

    // NOTE: we should never be looking at [len - 1].direction!
    visitedNodes[len].node = createdNode;
    len++;

    // Check for violations
    while (len >= 4 && visitedNodes[len - 2].node->color == RB_TREE_RED) {
        len =
            rebalanceInsert(visitedNodes[len - 3].direction, visitedNodes, len);
    }

    (*tree)->color = RB_TREE_BLACK;
}

// We have 2 subtrees hanbing from visitedNodes - 1, the subtree of direction
// and the subtree of !direction (other direction). the subtree of direction has
// 1 less black height than the other subtree. The potential tree above these
// subtrees is now also missing a black node. Fixing the deficiency by removing
// a black node from the other direction subtree means that we stil need to
// address that problem. On the other hand, coloring a node black in the
// direction subtree immediately solves the deficiency in the whole tree.
static U64 rebalanceDelete(RedBlackDirection direction,
                           VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                           U64 len) {
    RedBlackNodeBasic *node = visitedNodes[len - 1].node;
    RedBlackNodeBasic *childOtherDirection = node->children[!direction];
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

        visitedNodes[len - 1].node = childOtherDirection;
        visitedNodes[len].node = node;
        visitedNodes[len].direction = direction;
        len++;

        childOtherDirection = node->children[!direction];
    }

    RedBlackNodeBasic *innerChildOtherDirection =
        childOtherDirection->children[direction];
    RedBlackNodeBasic *outerChildOtherDirection =
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

        RedBlackNodeBasic *temp = childOtherDirection;
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

    return 0;
}

static RedBlackNodeBasic *
deleteNodeInPath(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U64 len,
                 RedBlackNodeBasic *toDelete) {
    U64 stepsToSuccessor = findAdjacentInSteps(
        (RedBlackNode *)toDelete, (CommonVisitedNode *)&visitedNodes[len],
        RB_TREE_RIGHT);
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!stepsToSuccessor) {
        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            toDelete->children[RB_TREE_LEFT];
    }
    // Swap the values of the node to delete with the values of the successor
    // node and delete the successor node instead (now containing the values of
    // the to delete node).
    else {
        U64 upperNodeIndex = len + 1;
        len += stepsToSuccessor;
        toDelete = visitedNodes[len - 1]
                       .node->children[visitedNodes[len - 1].direction];

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        U64 valueToKeep = toDelete->value;

        toDelete->value = visitedNodes[upperNodeIndex - 1].node->value;
        visitedNodes[upperNodeIndex - 1].node->value = valueToKeep;

        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            toDelete->children[RB_TREE_RIGHT];
    }

    // Fix the violations present by removing the toDelete node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (toDelete->color == RB_TREE_BLACK) {
        while (len >= 2) {
            RedBlackNodeBasic *childDeficitBlackDirection =
                visitedNodes[len - 1]
                    .node->children[visitedNodes[len - 1].direction];
            if (childDeficitBlackDirection &&
                childDeficitBlackDirection->color == RB_TREE_RED) {
                childDeficitBlackDirection->color = RB_TREE_BLACK;
                break;
            }

            len = rebalanceDelete(visitedNodes[len - 1].direction, visitedNodes,
                                  len);
        }
    }

    return toDelete;
}

RedBlackNodeBasic *deleteAtLeastRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                                  U64 value) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    U64 bestWithVisitedNodesLen = 0;

    for (RedBlackNodeBasic *potential = *tree; potential;) {
        if (potential->value == value) {
            return deleteNodeInPath(visitedNodes, len, potential);
        }

        if (potential->value > value) {
            bestWithVisitedNodesLen = len;
        }

        RedBlackDirection dir = calculateDirection(value, potential->value);
        visitedNodes[len].node = potential;
        visitedNodes[len].direction = dir;
        len++;

        potential = potential->children[dir];
    }

    if (bestWithVisitedNodesLen == 0) {
        return nullptr;
    }

    return deleteNodeInPath(visitedNodes, bestWithVisitedNodesLen,
                            visitedNodes[bestWithVisitedNodesLen].node);
}

// Assumes the value is inside the tree
RedBlackNodeBasic *deleteRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                           U64 value) {
    // Search
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    RedBlackNodeBasic *current = *tree;
    while (current->value != value) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction = calculateDirection(value, current->value);
        current = current->children[visitedNodes[len].direction];

        len++;
    }

    return deleteNodeInPath(visitedNodes, len, current);
}
