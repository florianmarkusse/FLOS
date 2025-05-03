#include "shared/trees/red-black-basic.h"
#include "shared/maths/maths.h"

typedef struct {
    RedBlackNodeBasic *node;
    RedBlackDirection direction;
} VisitedNode;

static U64 findAdjacentInSteps(RedBlackNodeBasic *node,
                               VisitedNode *visitedNodes,
                               RedBlackDirection direction) {
    if (!node->children[direction]) {
        return 0;
    }

    U64 traversals = 0;

    visitedNodes[traversals].node = node;
    visitedNodes[traversals].direction = direction;
    node = node->children[direction];
    traversals++;

    while (true) {
        RedBlackNodeBasic *next = node->children[!direction];
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

static RedBlackDirection calculateDirection(U64 value, U64 toCompare) {
    if (value >= toCompare) {
        return RB_TREE_RIGHT;
    }
    return RB_TREE_LEFT;
}

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
        parent->children[!direction] = node->children[direction];
        node->children[direction] = parent;
        grandParent->children[direction] = node;

        node = node->children[direction];
        parent = grandParent->children[direction];
    }

    //      x           y
    //     /           / \
    //    y      ==>  z   x
    //   /
    //  z
    grandParent->children[direction] = parent->children[!direction];
    parent->children[!direction] = grandParent;
    visitedNodes[len - 4].node->children[visitedNodes[len - 4].direction] =
        parent; // NOTE: Can also be that we are setting the new
                // root pointer here!

    parent->color = RB_TREE_BLACK;
    grandParent->color = RB_TREE_RED;

    return 0;
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
    //      x(B)              y(B)
    //       \               / \
    //        y(R)   ==>   x(R)a(B)
    //       / \            \
    //     z(B)a(B)        a(B)
    if (childOtherDirection->color == RB_TREE_RED) {
        childOtherDirection->color = RB_TREE_BLACK;
        node->color = RB_TREE_RED;

        node->children[!direction] = childOtherDirection->children[direction];
        childOtherDirection->children[direction] = node;
        visitedNodes[len - 2].node->children[visitedNodes[len - 2].direction] =
            childOtherDirection;

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

    //      x                     x
    //       \                     \
    //        y(B)          ===>   z(B)
    //       /                       \
    //     z(R)                      y(R)
    if ((!outerChildOtherDirection) ||
        outerChildOtherDirection->color == RB_TREE_BLACK) {
        childOtherDirection->color = RB_TREE_RED;
        innerChildOtherDirection->color = RB_TREE_BLACK;

        childOtherDirection->children[direction] =
            innerChildOtherDirection->children[!direction];
        innerChildOtherDirection->children[!direction] = childOtherDirection;
        node->children[!direction] = innerChildOtherDirection;

        RedBlackNodeBasic *temp = childOtherDirection;
        childOtherDirection = innerChildOtherDirection;
        outerChildOtherDirection = temp;
    }

    //      x                          y
    //     / \                        / \
    //   a(B)y(B)        ===>       x(B)z(B)
    //         \                    /
    //        z(R)                a(B)
    childOtherDirection->color = node->color;
    node->color = RB_TREE_BLACK;
    outerChildOtherDirection->color = RB_TREE_BLACK;

    node->children[!direction] = childOtherDirection->children[direction];
    childOtherDirection->children[direction] = node;

    visitedNodes[len - 2].node->children[visitedNodes[len - 2].direction] =
        childOtherDirection;

    return 0;
}

static RedBlackNodeBasic *
deleteNodeInPath(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U64 len,
                 RedBlackNodeBasic *toDelete) {
    U64 stepsToSuccessor =
        findAdjacentInSteps(toDelete, &visitedNodes[len], RB_TREE_RIGHT);
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
        U64 foundNodeIndex = len;
        len += stepsToSuccessor;
        toDelete = visitedNodes[len - 1]
                       .node->children[visitedNodes[len - 1].direction];

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        U64 foundValue = visitedNodes[foundNodeIndex].node->value;

        visitedNodes[foundNodeIndex].node->value = toDelete->value;
        toDelete->value = foundValue;

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
