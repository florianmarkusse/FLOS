#include "shared/trees/red-black/basic.h"
#include "shared/maths.h"

static RedBlackNodeBasic *getBasicNode(NodeLocation *nodeLocation, U32 index) {
    return (RedBlackNodeBasic *)getNode(nodeLocation, index);
}

static void initVisitedNodes(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                             U32 *len) {
    visitedNodes[0].node = 0;
    // NOTE: we should never be looking at [0].direction!
    *len = 1;
}

RedBlackNodeBasic *findGreatestBelowOrEqual(NodeLocation *nodeLocation,
                                            U32 *tree, U64 value) {
    U32 current = *tree;
    U32 result = 0;

    while (current) {
        RedBlackNodeBasic *currentPtr = getBasicNode(nodeLocation, current);
        if (currentPtr->value == value) {
            return currentPtr;
        } else if (currentPtr->value < value) {
            result = current;
            current = currentPtr->header.children[RB_TREE_RIGHT];
        } else {
            current = currentPtr->header.children[RB_TREE_LEFT];
        }
    }

    return getBasicNode(nodeLocation, result);
}

void insertRedBlackNodeBasic(NodeLocation *nodeLocation, U32 *tree,
                             RedBlackNodeBasic *createdNode) {
    createdNode->header.children[RB_TREE_LEFT] = 0;
    createdNode->header.children[RB_TREE_RIGHT] = 0;

    U32 createdNodeIndex = getIndex(nodeLocation, createdNode);
    if (!(*tree)) {
        // NOTE: Set created node to black, is already done by setting children
        // to 0
        *tree = createdNodeIndex;
        return;
    }

    // Search
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 current = *tree;
    RedBlackNodeBasic *currentNode = getBasicNode(nodeLocation, current);
    while (1) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction =
            calculateDirection(createdNode->value, currentNode->value);
        len++;

        U32 next =
            currentNode->header.children[visitedNodes[len - 1].direction];
        if (!next) {
            break;
        }
        current = next;
    }

    // Insert
    setColorWithPointer(&createdNode->header, RB_TREE_RED);
    currentNode->header.children[visitedNodes[len - 1].direction] =
        createdNodeIndex;

    visitedNodes[len].node = createdNodeIndex;
    // NOTE: we should never be looking at [len - 1].direction!
    len++;

    // Check for violations
    while (len >= 4 &&
           getColor(nodeLocation, visitedNodes[len - 2].node) == RB_TREE_RED) {
        len =
            rebalanceInsert(nodeLocation, tree, visitedNodes[len - 3].direction,
                            (CommonVisitedNode *)visitedNodes, len, nullptr);
    }

    setColor(nodeLocation, *tree, RB_TREE_BLACK);
}

static RedBlackNodeBasic *
deleteNodeInPath(NodeLocation *nodeLocation, U32 *tree,
                 VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                 U32 toDelete) {
    U32 stepsToSuccessor = findAdjacentInSteps(
        nodeLocation, toDelete, (CommonVisitedNode *)&visitedNodes[len],
        RB_TREE_RIGHT);

    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!stepsToSuccessor) {
        getBasicNode(nodeLocation, visitedNodes[len - 1].node)
            ->header.children[visitedNodes[len - 1].direction] =
            getBasicNode(nodeLocation, visitedNodes[len - 1].node)
                ->header.children[RB_TREE_LEFT];
    }
    // Swap the values of the node to delete with the values of the successor
    // node and delete the successor node instead (now containing the values of
    // the to delete node).
    else {
        U32 upperNodeIndex = len + 1;
        len += stepsToSuccessor;
        toDelete = getBasicNode(nodeLocation, visitedNodes[len - 1].node)
                       ->header.children[visitedNodes[len - 1].direction];

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        RedBlackNodeBasic *toDeleteNode = getBasicNode(nodeLocation, toDelete);
        RedBlackNodeBasic *swapNode =
            getBasicNode(nodeLocation, visitedNodes[upperNodeIndex - 1].node);

        U64 valueToKeep = toDeleteNode->value;

        toDeleteNode->value = swapNode->value;
        swapNode->value = valueToKeep;

        getBasicNode(nodeLocation, visitedNodes[len - 1].node)
            ->header.children[visitedNodes[len - 1].direction] =
            toDeleteNode->header.children[RB_TREE_RIGHT];
    }

    RedBlackNodeBasic *toDeleteNode = getBasicNode(nodeLocation, toDelete);
    // Fix the violations present by removing the toDelete node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (getColorWithPointer(&toDeleteNode->header) == RB_TREE_BLACK) {
        while (len >= 2) {
            U32 childDeficitBlackDirection =
                getBasicNode(nodeLocation, visitedNodes[len - 1].node)
                    ->header.children[visitedNodes[len - 1].direction];
            RedBlackNodeBasic *childDeficitBlackDirectionNode =
                getBasicNode(nodeLocation, childDeficitBlackDirection);
            if (childDeficitBlackDirection &&
                getColorWithPointer(&childDeficitBlackDirectionNode->header) ==
                    RB_TREE_RED) {
                setColorWithPointer(&childDeficitBlackDirectionNode->header,
                                    RB_TREE_BLACK);
                break;
            }

            len = rebalanceDelete(
                nodeLocation, tree, visitedNodes[len - 1].direction,
                (CommonVisitedNode *)visitedNodes, len, nullptr);
        }
    }

    return toDeleteNode;
}

RedBlackNodeBasic *deleteAtLeastRedBlackNodeBasic(NodeLocation *nodeLocation,
                                                  U32 *tree, U64 value) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 bestWithVisitedNodesLen = 0;

    U32 potentialIndex = *tree;
    for (RedBlackNodeBasic *potential = getBasicNode(nodeLocation, *tree);
         potential;) {
        if (potential->value == value) {
            return deleteNodeInPath(nodeLocation, tree, visitedNodes, len,
                                    potentialIndex);
        }

        if (potential->value > value) {
            bestWithVisitedNodesLen = len;
        }

        RedBlackDirection dir = calculateDirection(value, potential->value);
        visitedNodes[len].node = potentialIndex;
        visitedNodes[len].direction = dir;
        len++;

        potentialIndex = potential->header.children[dir];
        potential = getBasicNode(nodeLocation, potentialIndex);
    }

    if (bestWithVisitedNodesLen == 0) {
        return nullptr;
    }

    return deleteNodeInPath(nodeLocation, tree, visitedNodes,
                            bestWithVisitedNodesLen,
                            visitedNodes[bestWithVisitedNodesLen].node);
}

RedBlackNodeBasic *deleteRedBlackNodeBasic(NodeLocation *nodeLocation,
                                           U32 *tree, U64 value) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 currentIndex = *tree;
    for (RedBlackNodeBasic *current = getBasicNode(nodeLocation, currentIndex);
         current->value != value; len++) {
        visitedNodes[len].node = currentIndex;
        visitedNodes[len].direction = calculateDirection(value, current->value);

        currentIndex = current->header.children[visitedNodes[len].direction];
        current = getBasicNode(nodeLocation, currentIndex);
    }

    return deleteNodeInPath(nodeLocation, tree, visitedNodes, len,
                            currentIndex);
}
