#include "shared/trees/red-black/basic.h"
#include "shared/maths.h"

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
    U32 len = 1;

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
        len = rebalanceInsert(visitedNodes[len - 3].direction, visitedNodes,
                              len, nullptr);
    }

    (*tree)->color = RB_TREE_BLACK;
}

static RedBlackNodeBasic *
deleteNodeInPath(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                 RedBlackNodeBasic *toDelete) {
    U32 stepsToSuccessor = findAdjacentInSteps(
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
        U32 upperNodeIndex = len + 1;
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
                                  len, nullptr);
        }
    }

    return toDelete;
}

RedBlackNodeBasic *deleteAtLeastRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                                  U64 value) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U32 len = 1;

    U32 bestWithVisitedNodesLen = 0;

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

RedBlackNodeBasic *popRedBlackNodeBasic(RedBlackNodeBasic **tree) {
    if (!(*tree)) {
        return nullptr;
    }

    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U32 len = 1;

    return deleteNodeInPath(visitedNodes, len, *tree);
}

// Assumes the value is inside the tree
RedBlackNodeBasic *deleteRedBlackNodeBasic(RedBlackNodeBasic **tree,
                                           U64 value) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeBasic *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U32 len = 1;

    RedBlackNodeBasic *current = *tree;
    while (current->value != value) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction = calculateDirection(value, current->value);
        current = current->children[visitedNodes[len].direction];

        len++;
    }

    return deleteNodeInPath(visitedNodes, len, current);
}
