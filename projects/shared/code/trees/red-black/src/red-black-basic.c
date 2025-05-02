#include "shared/trees/red-black-basic.h"
#include "shared/maths/maths.h"

typedef struct {
    RedBlackNodeBasic *node;
    RedBlackDirection direction;
} VisitedNode;

typedef ARRAY(VisitedNode) VisitedNode_a;

static RedBlackDirection calculateDirection(U64 value,
                                            RedBlackNodeBasic *toCompare) {
    if (value >= toCompare->value) {
        return RB_TREE_RIGHT;
    }
    return RB_TREE_LEFT;
}

static U64 rebalanceInsert(RedBlackDirection direction, VisitedNode_a visited) {
    RedBlackNodeBasic *grandParent = visited.buf[visited.len - 3].node;
    RedBlackNodeBasic *parent = visited.buf[visited.len - 2].node;
    RedBlackNodeBasic *node = visited.buf[visited.len - 1].node;

    RedBlackNodeBasic *uncle = grandParent->children[!direction];
    if (uncle && uncle->color == RB_TREE_RED) {
        uncle->color = RB_TREE_BLACK;
        parent->color = RB_TREE_BLACK;
        grandParent->color = RB_TREE_RED;

        return visited.len - 2;
    }

    //      x             x
    //     /             /
    //    y       ==>   z
    //     \           /
    //      z         y
    if (visited.buf[visited.len - 2].direction == !direction) {
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
    visited.buf[visited.len - 4]
        .node->children[visited.buf[visited.len - 4].direction] =
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
    VisitedNode _visitedNodes[RB_TREE_MAX_HEIGHT];
    _visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    _visitedNodes[0].direction = RB_TREE_LEFT;
    VisitedNode_a visited = (VisitedNode_a){.buf = _visitedNodes, .len = 1};

    RedBlackNodeBasic *current = *tree;
    while (1) {
        visited.buf[visited.len].node = current;
        visited.buf[visited.len].direction =
            calculateDirection(createdNode->value, current);
        visited.len++;

        RedBlackNodeBasic *next =
            current->children[visited.buf[visited.len - 1].direction];
        if (!next) {
            break;
        }
        current = next;
    }

    // Insert
    createdNode->color = RB_TREE_RED;
    current->children[visited.buf[visited.len - 1].direction] = createdNode;

    // NOTE: we should never be looking at [len - 1].direction!
    visited.buf[visited.len].node = createdNode;
    visited.len++;

    // Check for violations
    while (visited.len >= 4 &&
           visited.buf[visited.len - 2].node->color == RB_TREE_RED) {
        visited.len =
            rebalanceInsert(visited.buf[visited.len - 3].direction, visited);
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
static U64 rebalanceDelete(RedBlackDirection direction, VisitedNode_a visited) {
    RedBlackNodeBasic *node = visited.buf[visited.len - 1].node;
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
        visited.buf[visited.len - 2]
            .node->children[visited.buf[visited.len - 2].direction] =
            childOtherDirection;

        visited.buf[visited.len - 1].node = childOtherDirection;
        visited.buf[visited.len].node = node;
        visited.buf[visited.len].direction = direction;
        visited.len++;

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

        return visited.len - 1;
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

    visited.buf[visited.len - 2]
        .node->children[visited.buf[visited.len - 2].direction] =
        childOtherDirection;

    return 0;
}

static RedBlackNodeBasic *deleteNodeInPath(VisitedNode_a visited,
                                           RedBlackNodeBasic *toDelete) {
    //    VisitedRedBlackNode_a succ = successor(
    //        (RedBlackNode *)toDelete,
    //        (VisitedRedBlackNode_a){.buf = (VisitedRedBlackNode *)visited.buf,
    //                                .len = visited.len});
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!(toDelete->children[RB_TREE_RIGHT])) {
        visited.buf[visited.len - 1]
            .node->children[visited.buf[visited.len - 1].direction] =
            toDelete->children[RB_TREE_LEFT];
    }
    // Find the sucessor node in the subtree of toDelete's right child. Done by
    // repeetedly going to the left child.
    else {
        visited.buf[visited.len].node = toDelete;
        visited.buf[visited.len].direction = RB_TREE_RIGHT;
        U64 foundNodeIndex = visited.len;
        toDelete = toDelete->children[RB_TREE_RIGHT];
        visited.len++;

        while (true) {
            RedBlackNodeBasic *next = toDelete->children[RB_TREE_LEFT];
            if (!next) {
                break;
            }

            visited.buf[visited.len].node = toDelete;
            visited.buf[visited.len].direction = RB_TREE_LEFT;
            visited.len++;

            toDelete = next;
        }

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        U64 foundValue = visited.buf[foundNodeIndex].node->value;

        visited.buf[foundNodeIndex].node->value = toDelete->value;
        toDelete->value = foundValue;

        visited.buf[visited.len - 1]
            .node->children[visited.buf[visited.len - 1].direction] =
            toDelete->children[RB_TREE_RIGHT];
    }

    // Fix the violations present by removing the toDelete node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (toDelete->color == RB_TREE_BLACK) {
        while (visited.len >= 2) {
            RedBlackNodeBasic *childDeficitBlackDirection =
                visited.buf[visited.len - 1]
                    .node->children[visited.buf[visited.len - 1].direction];
            if (childDeficitBlackDirection &&
                childDeficitBlackDirection->color == RB_TREE_RED) {
                childDeficitBlackDirection->color = RB_TREE_BLACK;
                break;
            }

            visited.len = rebalanceDelete(
                visited.buf[visited.len - 1].direction, visited);
        }
    }

    return toDelete;
}

RedBlackNodeBasic *deleteAtLeastRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                                  U64 value) {
    VisitedNode _visitedNodes[RB_TREE_MAX_HEIGHT];
    _visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    _visitedNodes[0].direction = RB_TREE_LEFT;
    VisitedNode_a visited = {.buf = _visitedNodes, .len = 1};

    U64 bestWithVisitedNodesLen = 0;

    for (RedBlackNodeBasic *potential = *tree; potential;) {
        if (potential->value == value) {
            return deleteNodeInPath(visited, potential);
        }

        if (potential->value > value) {
            bestWithVisitedNodesLen = visited.len;
        }

        RedBlackDirection dir = calculateDirection(value, potential);
        visited.buf[visited.len].node = potential;
        visited.buf[visited.len].direction = dir;
        visited.len++;

        potential = potential->children[dir];
    }

    if (bestWithVisitedNodesLen == 0) {
        return nullptr;
    }

    visited.len = bestWithVisitedNodesLen;
    return deleteNodeInPath(visited, visited.buf[visited.len].node);
}

// Assumes the value is inside the tree
RedBlackNodeBasic *deleteRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                           U64 value) {
    VisitedNode _visitedNodes[RB_TREE_MAX_HEIGHT];
    _visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    _visitedNodes[0].direction = RB_TREE_LEFT;
    VisitedNode_a visited = {.buf = _visitedNodes, .len = 1};

    RedBlackNodeBasic *current = *tree;
    while (current->value != value) {
        visited.buf[visited.len].node = current;
        visited.buf[visited.len].direction = calculateDirection(value, current);
        current = current->children[visited.buf[visited.len].direction];

        visited.len++;
    }

    return deleteNodeInPath(visited, current);
}
