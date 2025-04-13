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
    if (value >= toCompare->value) {
        return RIGHT_CHILD;
    }
    return LEFT_CHILD;
}

static U64 rebalanceInsert(RedBlackDirection direction,
                           VisitedNode visitedNodes[MAX_HEIGHT], U64 len) {
    RedBlackNode *grandParent = visitedNodes[len - 3].node;
    RedBlackNode *parent = visitedNodes[len - 2].node;
    RedBlackNode *node = visitedNodes[len - 1].node;

    RedBlackNode *uncle = grandParent->children[!direction];
    if (uncle && uncle->color == RED) {
        uncle->color = BLACK;
        parent->color = BLACK;
        grandParent->color = RED;

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

    grandParent->color = RED;
    parent->color = BLACK;

    return 0;
}

void insertRedBlackNode(RedBlackNode **tree, RedBlackNode *createdNode) {
    createdNode->children[LEFT_CHILD] = nullptr;
    createdNode->children[RIGHT_CHILD] = nullptr;

    if (!(*tree)) {
        createdNode->color = BLACK;
        *tree = createdNode;
        return;
    }

    // Search
    VisitedNode visitedNodes[MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNode *)tree;
    visitedNodes[0].direction = LEFT_CHILD;
    U64 len = 1;

    RedBlackNode *current = *tree;
    while (1) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction =
            calculateDirection(createdNode->value, current);
        len++;

        RedBlackNode *next = current->children[visitedNodes[len - 1].direction];
        if (!next) {
            break;
        }
        current = next;
    }

    // Insert
    createdNode->color = RED;
    current->children[visitedNodes[len - 1].direction] = createdNode;

    // NOTE: we should never be looking at [len - 1].direction!
    visitedNodes[len].node = createdNode;
    len++;

    // Check for violations
    while (len >= 4 && visitedNodes[len - 2].node->color == RED) {
        len =
            rebalanceInsert(visitedNodes[len - 3].direction, visitedNodes, len);
    }

    (*tree)->color = BLACK;
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
    if (childOtherDirection->color == RED) {
        childOtherDirection->color = BLACK;
        node->color = RED;

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
         innerChildOtherDirection->color == BLACK) &&
        ((!outerChildOtherDirection) ||
         outerChildOtherDirection->color == BLACK)) {
        childOtherDirection->color = RED;

        return len - 1;
    }

    //      x
    //       \
    //        y
    //         \
    //      (NOT RED)
    if ((!outerChildOtherDirection) ||
        outerChildOtherDirection->color == BLACK) {
        childOtherDirection->color = RED;
        innerChildOtherDirection->color = BLACK;

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
    node->color = BLACK;
    outerChildOtherDirection->color = BLACK;

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
    visitedNodes[0].direction = LEFT_CHILD;
    U64 len = 1;

    RedBlackNode *current = *tree;
    while (current->value != value) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction = calculateDirection(value, current);
        current = current->children[visitedNodes[len].direction];

        len++;
    }

    if (!(current->children[RIGHT_CHILD])) {
        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            current->children[LEFT_CHILD];
    } else {
        visitedNodes[len].node = current;
        visitedNodes[len].direction = RIGHT_CHILD;
        U64 foundNodeIndex = len;
        current = current->children[visitedNodes[len].direction];
        len++;

        while (true) {
            RedBlackNode *next = current->children[LEFT_CHILD];
            if (!next) {
                break;
            }

            visitedNodes[len].node = current;
            visitedNodes[len].direction = LEFT_CHILD;
            len++;

            current = next;
        }

        U64 foundNodeValue = visitedNodes[foundNodeIndex].node->value;
        visitedNodes[foundNodeIndex].node->value = current->value;
        current->value = foundNodeValue;

        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            current->children[RIGHT_CHILD];
    }

    if (current->color == BLACK) {
        while (len >= 2) {
            RedBlackNode *childDeficitBlackDirection =
                visitedNodes[len - 1]
                    .node->children[visitedNodes[len - 1].direction];
            if (childDeficitBlackDirection &&
                childDeficitBlackDirection->color == RED) {
                childDeficitBlackDirection->color = BLACK;
                break;
            }

            len = rebalanceDelete(visitedNodes[len - 1].direction, visitedNodes,
                                  len);
        }
    }

    return current;
}

RedBlackNode *findRedBlackNodeLeastBiggestValue(RedBlackNode *tree, U64 value) {
    // do stuff
    //
    //
}
