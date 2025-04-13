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

static bool rebalanceInsert(RedBlackDirection direction,
                            VisitedNode visitedNodes[MAX_HEIGHT], U64 len) {
    RedBlackNode *grandParent = visitedNodes[len - 3].node;
    RedBlackNode *parent = visitedNodes[len - 2].node;
    RedBlackNode *node = visitedNodes[len - 1].node;

    RedBlackNode *uncle = grandParent->children[!direction];
    if (uncle && uncle->color == RED) {
        uncle->color = BLACK;
        parent->color = BLACK;
        grandParent->color = RED;

        return false;
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

    return true;
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
        if (rebalanceInsert(visitedNodes[len - 3].direction, visitedNodes,
                            len)) {
            break;
        }
        len -= 2;
    }

    (*tree)->color = BLACK;
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
        while (true) {
            RedBlackNode *node = visitedNodes[len - 1].node;
            RedBlackNode *childDeficitBlackDirection =
                node->children[visitedNodes[len - 1].direction];

            if (childDeficitBlackDirection &&
                childDeficitBlackDirection->color == RED) {
                childDeficitBlackDirection->color = BLACK;
                break;
            }

            if (len < 2) {
                // Just the tree node left
                break;
            }

            RedBlackNode *childOtherDirection =
                node->children[!(visitedNodes[len - 1].direction)];
            if (visitedNodes[len - 1].direction == LEFT_CHILD) {
                if (childOtherDirection->color == RED) {
                    childOtherDirection->color = BLACK;
                    node->color = RED;

                    node->children[RIGHT_CHILD] =
                        childOtherDirection->children[LEFT_CHILD];
                    childOtherDirection->children[LEFT_CHILD] = node;
                    visitedNodes[len - 2]
                        .node->children[visitedNodes[len - 2].direction] =
                        childOtherDirection;

                    visitedNodes[len - 1].node = childOtherDirection;
                    visitedNodes[len].node = node;
                    visitedNodes[len].direction = LEFT_CHILD;
                    len++;

                    childOtherDirection = node->children[RIGHT_CHILD];
                }

                RedBlackNode *leftChildOtherDirection =
                    childOtherDirection->children[LEFT_CHILD];
                RedBlackNode *rightChildOtherDirection =
                    childOtherDirection->children[RIGHT_CHILD];
                if (((!leftChildOtherDirection) ||
                     leftChildOtherDirection->color == BLACK) &&
                    ((!rightChildOtherDirection) ||
                     rightChildOtherDirection->color == BLACK)) {
                    childOtherDirection->color = RED;
                } else {
                    if ((!rightChildOtherDirection) ||
                        rightChildOtherDirection->color == BLACK) {
                        childOtherDirection->color = RED;
                        leftChildOtherDirection->color = BLACK;

                        childOtherDirection->children[LEFT_CHILD] =
                            leftChildOtherDirection->children[RIGHT_CHILD];
                        leftChildOtherDirection->children[RIGHT_CHILD] =
                            childOtherDirection;
                        node->children[RIGHT_CHILD] = leftChildOtherDirection;

                        RedBlackNode *temp = childOtherDirection;
                        childOtherDirection = leftChildOtherDirection;
                        rightChildOtherDirection = temp;
                    }

                    childOtherDirection->color = node->color;
                    node->color = BLACK;
                    rightChildOtherDirection->color = BLACK;

                    node->children[RIGHT_CHILD] =
                        childOtherDirection->children[LEFT_CHILD];
                    childOtherDirection->children[LEFT_CHILD] = node;
                    visitedNodes[len - 2]
                        .node->children[visitedNodes[len - 2].direction] =
                        childOtherDirection;

                    break;
                }
            } else {
                if (childOtherDirection->color == RED) {
                    childOtherDirection->color = BLACK;
                    node->color = RED;

                    node->children[LEFT_CHILD] =
                        childOtherDirection->children[RIGHT_CHILD];
                    childOtherDirection->children[RIGHT_CHILD] = node;
                    visitedNodes[len - 2]
                        .node->children[visitedNodes[len - 2].direction] =
                        childOtherDirection;

                    visitedNodes[len - 1].node = childOtherDirection;
                    visitedNodes[len].node = node;
                    visitedNodes[len].direction = RIGHT_CHILD;
                    len++;

                    childOtherDirection = node->children[LEFT_CHILD];
                }

                RedBlackNode *rightChildOtherDirection =
                    childOtherDirection->children[RIGHT_CHILD];
                RedBlackNode *leftChildOtherDirection =
                    childOtherDirection->children[LEFT_CHILD];
                if (((!rightChildOtherDirection) ||
                     rightChildOtherDirection->color == BLACK) &&
                    ((!leftChildOtherDirection) ||
                     leftChildOtherDirection->color == BLACK)) {
                    childOtherDirection->color = RED;
                } else {
                    if ((!leftChildOtherDirection) ||
                        leftChildOtherDirection->color == BLACK) {
                        childOtherDirection->color = RED;
                        rightChildOtherDirection->color = BLACK;

                        childOtherDirection->children[RIGHT_CHILD] =
                            rightChildOtherDirection->children[LEFT_CHILD];
                        rightChildOtherDirection->children[LEFT_CHILD] =
                            childOtherDirection;
                        node->children[LEFT_CHILD] = rightChildOtherDirection;

                        RedBlackNode *temp = childOtherDirection;
                        childOtherDirection = rightChildOtherDirection;
                        leftChildOtherDirection = temp;
                    }

                    childOtherDirection->color = node->color;
                    node->color = BLACK;
                    leftChildOtherDirection->color = BLACK;

                    node->children[LEFT_CHILD] =
                        childOtherDirection->children[RIGHT_CHILD];
                    childOtherDirection->children[RIGHT_CHILD] = node;
                    visitedNodes[len - 2]
                        .node->children[visitedNodes[len - 2].direction] =
                        childOtherDirection;

                    break;
                }
            }

            len--;
        }
    }

    return current;
}

RedBlackNode *findRedBlackNodeLeastBiggestValue(RedBlackNode *tree, U64 value) {
    // do stuff
    //
    //
}
