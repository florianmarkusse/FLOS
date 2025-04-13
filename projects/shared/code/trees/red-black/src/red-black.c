#include "shared/trees/red-black.h"
#include "shared/memory/allocator/macros.h"
#include "shared/types/array.h"

static constexpr auto MAX_HEIGHT = 128;

typedef struct {
    RedBlackNode *node;
    RedBlackDirection direction;
} VisitedNode;

static RedBlackDirection calculateDirection(U64 value,
                                            RedBlackNode *toCompare) {
    if (value >= toCompare->bytes) {
        return RB_TREE_RIGHT;
    }
    return RB_TREE_LEFT;
}

static U64 rebalanceInsert(RedBlackDirection direction,
                           VisitedNode visitedNodes[MAX_HEIGHT], U64 len) {
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

    //      x
    //     /
    //    y
    //     \
    //      z
    if (visitedNodes[len - 2].direction == !direction) {
        parent->children[!direction] = node->children[direction];
        node->children[direction] = parent;
        grandParent->children[direction] = node;

        node = node->children[direction];
        parent = grandParent->children[direction];
    }

    //      x
    //     /
    //    y
    //   /
    //  z
    grandParent->children[direction] = parent->children[!direction];
    parent->children[!direction] = grandParent;
    visitedNodes[len - 4].node->children[visitedNodes[len - 4].direction] =
        parent; // NOTE: Can also be that we are setting the new
                // root pointer here!

    grandParent->color = RB_TREE_RED;
    parent->color = RB_TREE_BLACK;

    return 0;
}

void insertRedBlackNode(RedBlackNode **tree, RedBlackNode *createdNode) {
    createdNode->children[RB_TREE_LEFT] = nullptr;
    createdNode->children[RB_TREE_RIGHT] = nullptr;

    if (!(*tree)) {
        createdNode->color = RB_TREE_BLACK;
        *tree = createdNode;
        return;
    }

    // Search
    VisitedNode visitedNodes[MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNode *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    RedBlackNode *current = *tree;
    while (1) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction =
            calculateDirection(createdNode->bytes, current);
        len++;

        RedBlackNode *next = current->children[visitedNodes[len - 1].direction];
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
// address that proble. On the other hand, coloring a node black in the
// direction subtree immediately solves the deficiency in the whole tree.
static U64 rebalanceDelete(RedBlackDirection direction,
                           VisitedNode visitedNodes[MAX_HEIGHT], U64 len) {
    RedBlackNode *node = visitedNodes[len - 1].node;
    RedBlackNode *childOtherDirection = node->children[!direction];
    // Ensure the other child is colored black, we "push" the problem a level
    // down in the process.
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

    //      x
    //       \
    //        y
    //      /  \
    //   (RED) (NOT RED)
    if ((!outerChildOtherDirection) ||
        outerChildOtherDirection->color == RB_TREE_BLACK) {
        childOtherDirection->color = RB_TREE_RED;
        innerChildOtherDirection->color = RB_TREE_BLACK;

        childOtherDirection->children[direction] =
            innerChildOtherDirection->children[!direction];
        innerChildOtherDirection->children[!direction] = childOtherDirection;
        node->children[!direction] = innerChildOtherDirection;

        RedBlackNode *temp = childOtherDirection;
        childOtherDirection = innerChildOtherDirection;
        outerChildOtherDirection = temp;
    }

    //      x
    //       \
    //        y
    //         \
    //        (RED)
    childOtherDirection->color = node->color;
    node->color = RB_TREE_BLACK;
    outerChildOtherDirection->color = RB_TREE_BLACK;

    node->children[!direction] = childOtherDirection->children[direction];
    childOtherDirection->children[direction] = node;
    visitedNodes[len - 2].node->children[visitedNodes[len - 2].direction] =
        childOtherDirection;

    return 0;
}

// Assumes the value is inside the tree
RedBlackNode *deleteRedBlackNode(RedBlackNode **tree, U64 value) {
    // Search
    VisitedNode visitedNodes[MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNode *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    RedBlackNode *current = *tree;
    while (current->bytes != value) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction = calculateDirection(value, current);
        current = current->children[visitedNodes[len].direction];

        len++;
    }

    // If there is no right child, we can delete by having the parent of current
    // now point to current's left child instead of current.
    if (!(current->children[RB_TREE_RIGHT])) {
        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            current->children[RB_TREE_LEFT];
    }
    // Find the sucessor node in the subtree of current's left chld. Done by
    // repeetedly going to the left child.
    else {
        visitedNodes[len].node = current;
        visitedNodes[len].direction = RB_TREE_RIGHT;
        U64 foundNodeIndex = len;
        current = current->children[visitedNodes[len].direction];
        len++;

        while (true) {
            RedBlackNode *next = current->children[RB_TREE_LEFT];
            if (!next) {
                break;
            }

            visitedNodes[len].node = current;
            visitedNodes[len].direction = RB_TREE_LEFT;
            len++;

            current = next;
        }

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        U64 foundNodeBytes = visitedNodes[foundNodeIndex].node->bytes;
        U64 foundNodeStart = visitedNodes[foundNodeIndex].node->start;
        visitedNodes[foundNodeIndex].node->bytes = current->bytes;
        visitedNodes[foundNodeIndex].node->start = current->start;
        current->bytes = foundNodeBytes;
        current->start = foundNodeStart;

        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            current->children[RB_TREE_RIGHT];
    }

    // Fix the violations present by removing the current node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (current->color == RB_TREE_BLACK) {
        while (len >= 2) {
            RedBlackNode *childDeficitBlackDirection =
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

    return current;
}
