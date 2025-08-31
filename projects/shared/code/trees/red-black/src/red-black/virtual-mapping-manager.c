#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/converter.h"

VMMNode *getVMMNode(VMMTreeWithFreeList *treeWithFreeList, U32 index) {
    return (VMMNode *)getNode((TreeWithFreeList *)treeWithFreeList, index);
}

static U32 deleteNodeInPath(VMMTreeWithFreeList *treeWithFreeList,
                            U32 visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                            U32 toDelete) {
    U32 stepsToSuccessor =
        findAdjacentInSteps((TreeWithFreeList *)treeWithFreeList,
                            &visitedNodes[len], toDelete, RB_TREE_RIGHT);

    VMMNode *toDeleteNode = getVMMNode(treeWithFreeList, toDelete);
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!stepsToSuccessor) {
        U32 leftChildIndexToDeleteNode =
            childNodePointerGet(&toDeleteNode->header, RB_TREE_LEFT);
        U32 previousIndex = visitedNodeIndexGet(visitedNodes, len - 1);
        if (!previousIndex) {
            treeWithFreeList->rootIndex = leftChildIndexToDeleteNode;
        } else {
            VMMNode *parentNode = getVMMNode(treeWithFreeList, previousIndex);
            childNodePointerSet(&parentNode->header,
                                visitedNodeDirectionGet(visitedNodes, len - 1),
                                leftChildIndexToDeleteNode);
        }
    }
    // Swap the values of the node to delete with the values of the successor
    // node and delete the successor node instead (now containing the values of
    // the to delete node).
    else {
        U32 upperNodeIndex = len + 1;
        len += stepsToSuccessor;

        VMMNode *nodeGettingNewChild = getVMMNode(
            treeWithFreeList, visitedNodeIndexGet(visitedNodes, len - 1));
        toDelete =
            childNodePointerGet(&nodeGettingNewChild->header,
                                visitedNodeDirectionGet(visitedNodes, len - 1));

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        toDeleteNode = getVMMNode(treeWithFreeList, toDelete);
        VMMNode *swapNode =
            getVMMNode(treeWithFreeList,
                       visitedNodeIndexGet(visitedNodes, upperNodeIndex - 1));

        VMMData valueToKeep = toDeleteNode->data;

        toDeleteNode->data = swapNode->data;
        swapNode->data = valueToKeep;

        childNodePointerSet(
            &nodeGettingNewChild->header,
            visitedNodeDirectionGet(visitedNodes, len - 1),
            childNodePointerGet(&toDeleteNode->header, RB_TREE_RIGHT));
    }

    // Fix the violations present by removing the toDelete node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (getColorWithPointer(&toDeleteNode->header) == RB_TREE_BLACK) {
        while (len >= 2) {
            U32 childDeficitBlackDirection = childNodeIndexGet(
                (TreeWithFreeList *)treeWithFreeList,
                visitedNodeIndexGet(visitedNodes, len - 1),
                visitedNodeDirectionGet(visitedNodes, len - 1));
            VMMNode *childDeficitBlackDirectionNode =
                getVMMNode(treeWithFreeList, childDeficitBlackDirection);
            if (childDeficitBlackDirection &&
                getColorWithPointer(&childDeficitBlackDirectionNode->header) ==
                    RB_TREE_RED) {
                setColorWithPointer(&childDeficitBlackDirectionNode->header,
                                    RB_TREE_BLACK);
                break;
            }

            len = rebalanceDelete(
                (TreeWithFreeList *)treeWithFreeList, visitedNodes, len,
                visitedNodeDirectionGet(visitedNodes, len - 1), nullptr);
        }
    }

    return toDelete;
}

void insertVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                   VMMNode *createdNode) {
    // Zero out children (& color)
    memset(createdNode->header.metaData, 0,
           sizeof(createdNode->header.metaData));

    U32 createdNodeIndex =
        getIndex((TreeWithFreeList *)treeWithFreeList, createdNode);
    if (!(treeWithFreeList->rootIndex)) {
        // NOTE: Set created node to black, is already done by setting children
        // to 0
        treeWithFreeList->rootIndex = createdNodeIndex;
        return;
    }

    // Search
    U32 visitedNodes[RB_TREE_MAX_HEIGHT];
    visitedNodes[0] = 0;
    U32 len = 1;

    U32 current = treeWithFreeList->rootIndex;
    VMMNode *currentNode = getVMMNode(treeWithFreeList, current);
    while (1) {
        visitedNodeIndexSet(visitedNodes, len, current);
        RedBlackDirection dir = calculateDirection(
            createdNode->data.memory.start, currentNode->data.memory.start);
        visitedNodeDirectionSet(visitedNodes, len, dir);
        len++;

        U32 next = childNodePointerGet(&currentNode->header, dir);
        if (!next) {
            break;
        }

        current = next;
        currentNode = getVMMNode(treeWithFreeList, current);
    }

    // Insert
    setColorWithPointer(&createdNode->header, RB_TREE_RED);
    childNodePointerSet(&currentNode->header,
                        visitedNodeDirectionGet(visitedNodes, len - 1),
                        createdNodeIndex);

    visitedNodeIndexSet(visitedNodes, len, createdNodeIndex);
    len++;

    // Check for violations
    while (len >= 4 && getColor((TreeWithFreeList *)treeWithFreeList,
                                visitedNodeIndexGet(visitedNodes, len - 2)) ==
                           RB_TREE_RED) {
        len = rebalanceInsert(
            (TreeWithFreeList *)treeWithFreeList, visitedNodes, len,
            visitedNodeDirectionGet(visitedNodes, len - 3), nullptr);
    }

    setColor((TreeWithFreeList *)treeWithFreeList, treeWithFreeList->rootIndex,
             RB_TREE_BLACK);
}

U32 deleteVMMNode(VMMTreeWithFreeList *treeWithFreeList, U64 value) {
    U32 visitedNodes[RB_TREE_MAX_HEIGHT];
    visitedNodes[0] = 0;
    U32 len = 1;

    U32 currentIndex = treeWithFreeList->rootIndex;
    for (VMMNode *current = getVMMNode(treeWithFreeList, currentIndex);
         current->data.memory.start != value; len++) {
        visitedNodeIndexSet(visitedNodes, len, currentIndex);
        RedBlackDirection dir =
            calculateDirection(value, current->data.memory.start);
        visitedNodeDirectionSet(visitedNodes, len, dir);

        currentIndex = childNodePointerGet(&current->header, dir);
        current = getVMMNode(treeWithFreeList, currentIndex);
    }

    return deleteNodeInPath(treeWithFreeList, visitedNodes, len, currentIndex);
}

[[nodiscard]] U32 deleteAtLeastVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                                       U64 value) {
    U32 visitedNodes[RB_TREE_MAX_HEIGHT];
    visitedNodes[0] = 0;
    U32 len = 1;

    U32 bestWithVisitedNodesLen = 0;

    U32 potentialIndex = treeWithFreeList->rootIndex;
    while (potentialIndex) {
        VMMNode *potential = getVMMNode(treeWithFreeList, potentialIndex);
        if (potential->data.memory.start == value) {
            return deleteNodeInPath(treeWithFreeList, visitedNodes, len,
                                    potentialIndex);
        }

        if (potential->data.memory.start > value) {
            bestWithVisitedNodesLen = len;
        }

        RedBlackDirection dir =
            calculateDirection(value, potential->data.memory.start);
        visitedNodeIndexSet(visitedNodes, len, potentialIndex);
        visitedNodeDirectionSet(visitedNodes, len, dir);
        len++;

        potentialIndex = childNodePointerGet(&potential->header, dir);
    }

    if (bestWithVisitedNodesLen == 0) {
        return 0;
    }

    return deleteNodeInPath(
        treeWithFreeList, visitedNodes, bestWithVisitedNodesLen,
        visitedNodeIndexGet(visitedNodes, bestWithVisitedNodesLen));
}

VMMNode *findGreatestBelowOrEqualVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                                         U64 address) {
    U32 current = treeWithFreeList->rootIndex;
    U32 result = 0;

    while (current) {
        VMMNode *currentPtr = getVMMNode(treeWithFreeList, current);
        if (currentPtr->data.memory.start == address) {
            return currentPtr;
        } else if (currentPtr->data.memory.start < address) {
            result = current;
            current = childNodePointerGet(&currentPtr->header, RB_TREE_RIGHT);
        } else {
            current = childNodePointerGet(&currentPtr->header, RB_TREE_LEFT);
        }
    }

    return getVMMNode(treeWithFreeList, result);
}
