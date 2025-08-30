#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/converter.h"

VMMNode *getVMMNode(VMMTreeWithFreeList *treeWithFreeList, U32 index) {
    return (VMMNode *)getNode((TreeWithFreeList *)treeWithFreeList, index);
}

static void initVisitedNodes(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                             U32 *len) {
    visitedNodes[0].index = 0;
    // NOTE: we should never be looking at [0].direction!
    *len = 1;
}

static U32 deleteNodeInPath(VMMTreeWithFreeList *treeWithFreeList,
                            VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                            U32 len, U32 toDelete) {
    U32 stepsToSuccessor =
        findAdjacentInSteps((TreeWithFreeList *)treeWithFreeList,
                            &visitedNodes[len], toDelete, RB_TREE_RIGHT);

    VMMNode *toDeleteNode = getVMMNode(treeWithFreeList, toDelete);
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!stepsToSuccessor) {
        U32 leftChildIndexToDeleteNode =
            childNodePointerGet(&toDeleteNode->header, RB_TREE_LEFT);
        if (!visitedNodes[len - 1].index) {
            treeWithFreeList->tree = leftChildIndexToDeleteNode;
        } else {
            VMMNode *parentNode =
                getVMMNode(treeWithFreeList, visitedNodes[len - 1].index);
            childNodePointerSet(&parentNode->header,
                                visitedNodes[len - 1].direction,
                                leftChildIndexToDeleteNode);
        }
    }
    // Swap the values of the node to delete with the values of the successor
    // node and delete the successor node instead (now containing the values of
    // the to delete node).
    else {
        U32 upperNodeIndex = len + 1;
        len += stepsToSuccessor;

        VMMNode *nodeGettingNewChild =
            getVMMNode(treeWithFreeList, visitedNodes[len - 1].index);
        toDelete = childNodePointerGet(&nodeGettingNewChild->header,
                                       visitedNodes[len - 1].direction);

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        toDeleteNode = getVMMNode(treeWithFreeList, toDelete);
        VMMNode *swapNode = getVMMNode(treeWithFreeList,
                                       visitedNodes[upperNodeIndex - 1].index);

        VMMData valueToKeep = toDeleteNode->data;

        toDeleteNode->data = swapNode->data;
        swapNode->data = valueToKeep;

        childNodePointerSet(
            &nodeGettingNewChild->header, visitedNodes[len - 1].direction,
            childNodePointerGet(&toDeleteNode->header, RB_TREE_RIGHT));
    }

    // Fix the violations present by removing the toDelete node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (getColorWithPointer(&toDeleteNode->header) == RB_TREE_BLACK) {
        while (len >= 2) {
            U32 childDeficitBlackDirection = childNodeIndexGet(
                (TreeWithFreeList *)treeWithFreeList,
                visitedNodes[len - 1].index, visitedNodes[len - 1].direction);
            VMMNode *childDeficitBlackDirectionNode =
                getVMMNode(treeWithFreeList, childDeficitBlackDirection);
            if (childDeficitBlackDirection &&
                getColorWithPointer(&childDeficitBlackDirectionNode->header) ==
                    RB_TREE_RED) {
                setColorWithPointer(&childDeficitBlackDirectionNode->header,
                                    RB_TREE_BLACK);
                break;
            }

            len = rebalanceDelete((TreeWithFreeList *)treeWithFreeList,
                                  visitedNodes, len,
                                  visitedNodes[len - 1].direction, nullptr);
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
    if (!(treeWithFreeList->tree)) {
        // NOTE: Set created node to black, is already done by setting children
        // to 0
        treeWithFreeList->tree = createdNodeIndex;
        return;
    }

    // Search
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 current = treeWithFreeList->tree;
    VMMNode *currentNode = getVMMNode(treeWithFreeList, current);
    while (1) {
        visitedNodes[len].index = current;
        visitedNodes[len].direction = calculateDirection(
            createdNode->data.memory.start, currentNode->data.memory.start);
        len++;

        U32 next = childNodePointerGet(&currentNode->header,
                                       visitedNodes[len - 1].direction);
        if (!next) {
            break;
        }

        current = next;
        currentNode = getVMMNode(treeWithFreeList, current);
    }

    // Insert
    setColorWithPointer(&createdNode->header, RB_TREE_RED);
    childNodePointerSet(&currentNode->header, visitedNodes[len - 1].direction,
                        createdNodeIndex);

    // NOTE: we should never be looking at [len - 1].direction!
    visitedNodes[len].index = createdNodeIndex;
    len++;

    // Check for violations
    while (len >= 4 && getColor((TreeWithFreeList *)treeWithFreeList,
                                visitedNodes[len - 2].index) == RB_TREE_RED) {
        len =
            rebalanceInsert((TreeWithFreeList *)treeWithFreeList, visitedNodes,
                            len, visitedNodes[len - 3].direction, nullptr);
    }

    setColor((TreeWithFreeList *)treeWithFreeList, treeWithFreeList->tree,
             RB_TREE_BLACK);
}

U32 deleteVMMNode(VMMTreeWithFreeList *treeWithFreeList, U64 value) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 currentIndex = treeWithFreeList->tree;
    for (VMMNode *current = getVMMNode(treeWithFreeList, currentIndex);
         current->data.memory.start != value; len++) {
        visitedNodes[len].index = currentIndex;
        visitedNodes[len].direction =
            calculateDirection(value, current->data.memory.start);

        currentIndex =
            childNodePointerGet(&current->header, visitedNodes[len].direction);
        current = getVMMNode(treeWithFreeList, currentIndex);
    }

    return deleteNodeInPath(treeWithFreeList, visitedNodes, len, currentIndex);
}

[[nodiscard]] U32 deleteAtLeastVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                                       U64 value) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 bestWithVisitedNodesLen = 0;

    U32 potentialIndex = treeWithFreeList->tree;
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
        visitedNodes[len].index = potentialIndex;
        visitedNodes[len].direction = dir;
        len++;

        potentialIndex = childNodePointerGet(&potential->header, dir);
    }

    if (bestWithVisitedNodesLen == 0) {
        return 0;
    }

    return deleteNodeInPath(treeWithFreeList, visitedNodes,
                            bestWithVisitedNodesLen,
                            visitedNodes[bestWithVisitedNodesLen].index);
}

VMMNode *findGreatestBelowOrEqualVMMNode(VMMTreeWithFreeList *treeWithFreeList,
                                         U64 address) {
    U32 current = treeWithFreeList->tree;
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
