#include "shared/trees/red-black/memory-manager.h"
#include "abstraction/memory/manipulation.h"
#include "shared/maths.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/basic.h"
#include "shared/types/array.h"

MMNode *getMMNode(MMTreeWithFreeList *treeWithFreeList, U32 index) {
    return (MMNode *)getNode((TreeWithFreeList *)treeWithFreeList, index);
}

static void initVisitedNodes(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                             U32 *len) {
    visitedNodes[0].index = 0;
    // NOTE: we should never be looking at [0].direction!
    *len = 1;
}

static void recalculateMostBytes(MMTreeWithFreeList *treeWithFreeList,
                                 MMNode *node) {
    node->data.mostBytesInSubtree = node->data.memory.bytes;
    U32 rightChildIndex = childNodePointerGet(&node->header, RB_TREE_RIGHT);
    if (rightChildIndex) {
        MMNode *childNode = getMMNode(treeWithFreeList, rightChildIndex);
        node->data.mostBytesInSubtree = MAX(node->data.mostBytesInSubtree,
                                            childNode->data.mostBytesInSubtree);
    }
    U32 leftChildIndex = childNodePointerGet(&node->header, RB_TREE_LEFT);
    if (leftChildIndex) {
        MMNode *childNode = getMMNode(treeWithFreeList, leftChildIndex);
        node->data.mostBytesInSubtree = MAX(node->data.mostBytesInSubtree,
                                            childNode->data.mostBytesInSubtree);
    }
}

// The fist entry always contains the pointer to the address of the root node.
static constexpr auto ROOT_NODE_INDEX_LEN = 1;

static void propagateInsertUpwards(MMTreeWithFreeList *treeWithFreeList,
                                   VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                   U32 len, U64 newMostBytesInSubtree) {
    while (len > ROOT_NODE_INDEX_LEN) {
        MMNode *node = getMMNode(treeWithFreeList, visitedNodes[len - 1].index);
        if (node->data.mostBytesInSubtree >= newMostBytesInSubtree) {
            return;
        }
        node->data.mostBytesInSubtree = newMostBytesInSubtree;
        len--;
    }
}

static void propagateDeleteUpwards(MMTreeWithFreeList *treeWithFreeList,
                                   VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                   U32 len, U64 deletedBytes, U32 end) {
    while (len > end) {
        MMNode *node = getMMNode(treeWithFreeList, visitedNodes[len - 1].index);
        recalculateMostBytes(treeWithFreeList, node);
        if (node->data.mostBytesInSubtree >= deletedBytes) {
            return;
        }
        len--;
    }
}

static void setMostBytesAfterRotation(TreeWithFreeList *treeWithFreeList,
                                      U32 prevRotationNode,
                                      U32 prevRotationChild) {
    MMNode *previousRotationNode =
        getMMNode((MMTreeWithFreeList *)treeWithFreeList, prevRotationNode);
    MMNode *previousRotationChild =
        getMMNode((MMTreeWithFreeList *)treeWithFreeList, prevRotationChild);
    previousRotationChild->data.mostBytesInSubtree =
        previousRotationNode->data.mostBytesInSubtree;
    recalculateMostBytes((MMTreeWithFreeList *)treeWithFreeList,
                         previousRotationNode);
}

static U32 deleteNodeInPath(MMTreeWithFreeList *treeWithFreeList,
                            VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                            U32 len, U32 toDelete) {
    U32 stepsToSuccessor =
        findAdjacentInSteps((TreeWithFreeList *)treeWithFreeList,
                            &visitedNodes[len], toDelete, RB_TREE_RIGHT);

    MMNode *toDeleteNode = getMMNode(treeWithFreeList, toDelete);
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!stepsToSuccessor) {
        U32 leftChildIndexToDeleteNode =
            childNodePointerGet(&toDeleteNode->header, RB_TREE_LEFT);
        if (!visitedNodes[len - 1].index) {
            treeWithFreeList->tree = leftChildIndexToDeleteNode;
        } else {
            MMNode *parentNode =
                getMMNode(treeWithFreeList, visitedNodes[len - 1].index);
            childNodePointerSet(&parentNode->header,
                                visitedNodes[len - 1].direction,
                                leftChildIndexToDeleteNode);
            propagateDeleteUpwards(treeWithFreeList, visitedNodes, len,
                                   toDeleteNode->data.memory.bytes,
                                   ROOT_NODE_INDEX_LEN);
        }
    }
    // Swap the values of the node to delete with the values of the successor
    // node and delete the successor node instead (now containing the values
    // of the to delete node).
    else {
        U32 upperNodeIndex = len + 1;
        len += stepsToSuccessor;

        MMNode *nodeGettingNewChild =
            getMMNode(treeWithFreeList, visitedNodes[len - 1].index);
        toDelete = childNodePointerGet(&nodeGettingNewChild->header,
                                       visitedNodes[len - 1].direction);

        // Swap the values around. Naturally, the node pointers can be
        // swapped too. Just swapping memory here, mostBytesInSubtree will be
        // updated after.
        toDeleteNode = getMMNode(treeWithFreeList, toDelete);
        MMNode *swapNode =
            getMMNode(treeWithFreeList, visitedNodes[upperNodeIndex - 1].index);

        Memory memoryToKeep = toDeleteNode->data.memory;

        toDeleteNode->data.memory = swapNode->data.memory;
        swapNode->data.memory = memoryToKeep;

        childNodePointerSet(
            &nodeGettingNewChild->header, visitedNodes[len - 1].direction,
            childNodePointerGet(&toDeleteNode->header, RB_TREE_RIGHT));

        // In the first part, memoryToKeep got "deleted", i.e., moved higher
        // in
        // the subtree. When we reach the node where the memoryTokeep is now
        //   at,
        // toDelete->memory got deleted.
        propagateDeleteUpwards(treeWithFreeList, visitedNodes, len,
                               memoryToKeep.bytes, upperNodeIndex);
        propagateDeleteUpwards(treeWithFreeList, visitedNodes, upperNodeIndex,
                               toDeleteNode->data.memory.bytes,
                               ROOT_NODE_INDEX_LEN);
    }

    // Fix the violations present by removing the toDelete node. Note that
    // this node does not have to be the node that originally contained the
    // value to be deleted.
    if (getColorWithPointer(&toDeleteNode->header) == RB_TREE_BLACK) {
        while (len >= 2) {
            U32 childDeficitBlackDirection = childNodeIndexGet(
                (TreeWithFreeList *)treeWithFreeList,
                visitedNodes[len - 1].index, visitedNodes[len - 1].direction);
            MMNode *childDeficitBlackDirectionNode =
                getMMNode(treeWithFreeList, childDeficitBlackDirection);
            if (childDeficitBlackDirection &&
                getColorWithPointer(&childDeficitBlackDirectionNode->header) ==
                    RB_TREE_RED) {
                setColorWithPointer(&childDeficitBlackDirectionNode->header,
                                    RB_TREE_BLACK);
                break;
            }

            len = rebalanceDelete(
                (TreeWithFreeList *)treeWithFreeList, visitedNodes, len,
                visitedNodes[len - 1].direction, setMostBytesAfterRotation);
        }
    }

    return toDelete;
}

static void
propogateInsertIfRequired(MMTreeWithFreeList *treeWithFreeList,
                          VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                          MMNode *current) {
    if (current->data.memory.bytes > current->data.mostBytesInSubtree) {
        current->data.mostBytesInSubtree = current->data.memory.bytes;
        propagateInsertUpwards(treeWithFreeList, visitedNodes, len,
                               current->data.memory.bytes);
    }
}

static U32 bridgeMerge(MMTreeWithFreeList *treeWithFreeList,
                       VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                       MMNode *current, U64 adjacentBytes, U32 adjacentSteps) {
    current->data.memory.bytes += adjacentBytes;
    propogateInsertIfRequired(treeWithFreeList, visitedNodes, len, current);
    U32 newLen = len + adjacentSteps;

    U32 previousNewLen = newLen - 1;
    MMNode *previousNewParent =
        getMMNode(treeWithFreeList, visitedNodes[previousNewLen].index);

    return deleteNodeInPath(
        treeWithFreeList, visitedNodes, newLen,
        childNodePointerGet(&previousNewParent->header,
                            visitedNodes[previousNewLen].direction));
}

static U32 beforeRegionMerge(MMTreeWithFreeList *treeWithFreeList,
                             VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                             U32 len, U32 current, MMNode *createdNode) {
    MMNode *currentNode = getMMNode(treeWithFreeList, current);
    currentNode->data.memory.bytes += createdNode->data.memory.bytes;

    U32 predecessorSteps =
        findAdjacentInSteps((TreeWithFreeList *)treeWithFreeList,
                            &visitedNodes[len], current, RB_TREE_LEFT);
    if (predecessorSteps) {
        U32 predecessorLen = len + predecessorSteps - 1;
        MMNode *predecessorParent =
            getMMNode(treeWithFreeList, visitedNodes[predecessorLen].index);
        MMNode *predecessor = getMMNode(
            treeWithFreeList,
            childNodePointerGet(&predecessorParent->header,
                                visitedNodes[predecessorLen].direction));
        // | predecessor | created | current |
        if (predecessor->data.memory.start + predecessor->data.memory.bytes ==
            createdNode->data.memory.start) {
            currentNode->data.memory.start = predecessor->data.memory.start;
            return bridgeMerge(treeWithFreeList, visitedNodes, len, currentNode,
                               predecessor->data.memory.bytes,
                               predecessorSteps);
        }
    }

    currentNode->data.memory.start = createdNode->data.memory.start;
    propogateInsertIfRequired(treeWithFreeList, visitedNodes, len, currentNode);
    return 0;
}

static U32 afterRegionMerge(MMTreeWithFreeList *treeWithFreeList,
                            VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                            U32 len, U32 current, MMNode *createdNode,
                            U64 createdEnd) {
    MMNode *currentNode = getMMNode(treeWithFreeList, current);
    currentNode->data.memory.bytes += createdNode->data.memory.bytes;

    U32 successorSteps =
        findAdjacentInSteps((TreeWithFreeList *)treeWithFreeList,
                            &visitedNodes[len], current, RB_TREE_RIGHT);
    if (successorSteps) {
        U32 successorLen = len + successorSteps - 1;
        MMNode *successorParent =
            getMMNode(treeWithFreeList, visitedNodes[successorLen].index);
        MMNode *successor = getMMNode(
            treeWithFreeList,
            childNodePointerGet(&successorParent->header,
                                visitedNodes[successorLen].direction));
        // | current | created | successor |
        if (createdEnd == successor->data.memory.start) {
            return bridgeMerge(treeWithFreeList, visitedNodes, len, currentNode,
                               successor->data.memory.bytes, successorSteps);
        }
    }

    propogateInsertIfRequired(treeWithFreeList, visitedNodes, len, currentNode);

    return 0;
}

InsertResult insertMMNode(MMTreeWithFreeList *treeWithFreeList,
                          MMNode *createdNode) {
    InsertResult result = {.freed = {0}};

    // Zero out children (& color)
    memset(createdNode->header.metaData, 0,
           sizeof(createdNode->header.metaData));

    U32 createdNodeIndex =
        getIndex((TreeWithFreeList *)treeWithFreeList, createdNode);
    if (!(treeWithFreeList->tree)) {
        // NOTE: Set created node to black, is already done by setting children
        // to 0
        createdNode->data.mostBytesInSubtree = createdNode->data.memory.bytes;
        treeWithFreeList->tree = createdNodeIndex;
        return result;
    }

    // Search
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 current = treeWithFreeList->tree;
    MMNode *currentNode = getMMNode(treeWithFreeList, current);
    U64 createdEnd =
        createdNode->data.memory.start + createdNode->data.memory.bytes;
    while (1) {
        U64 currentEnd =
            currentNode->data.memory.start + currentNode->data.memory.bytes;
        // | created | current |
        if (createdEnd == currentNode->data.memory.start) {
            result.freed[0] =
                getIndex((TreeWithFreeList *)treeWithFreeList, createdNode);
            result.freed[1] = beforeRegionMerge(treeWithFreeList, visitedNodes,
                                                len, current, createdNode);
            return result;
        }
        // | current | created |
        else if (currentEnd == createdNode->data.memory.start) {
            result.freed[0] = createdNodeIndex;
            result.freed[1] =
                afterRegionMerge(treeWithFreeList, visitedNodes, len, current,
                                 createdNode, createdEnd);
            return result;
        }

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
        currentNode = getMMNode(treeWithFreeList, current);
    }

    // Insert
    setColorWithPointer(&createdNode->header, RB_TREE_RED);
    createdNode->data.mostBytesInSubtree = createdNode->data.memory.bytes;
    childNodePointerSet(&currentNode->header, visitedNodes[len - 1].direction,
                        createdNodeIndex);
    propagateInsertUpwards(treeWithFreeList, visitedNodes, len,
                           createdNode->data.memory.bytes);

    // NOTE: we should never be looking at [len - 1].direction!
    visitedNodes[len].index = createdNodeIndex;
    len++;

    // Check for violations
    while (len >= 4 && getColor((TreeWithFreeList *)treeWithFreeList,
                                visitedNodes[len - 2].index) == RB_TREE_RED) {
        len = rebalanceInsert(
            (TreeWithFreeList *)treeWithFreeList, visitedNodes, len,
            visitedNodes[len - 3].direction, setMostBytesAfterRotation);
    }

    setColor((TreeWithFreeList *)treeWithFreeList, treeWithFreeList->tree,
             RB_TREE_BLACK);

    return result;
}

U32 deleteAtLeastMMNode(MMTreeWithFreeList *treeWithFreeList, U64 bytes) {
    if (!treeWithFreeList->tree) {
        return 0;
    }

    MMNode *treeNode = getMMNode(treeWithFreeList, treeWithFreeList->tree);
    if (treeNode->data.mostBytesInSubtree < bytes) {
        return 0;
    }

    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U64 bestSoFar = treeNode->data.mostBytesInSubtree;
    U32 bestWithVisitedNodesLen = 0;

    U32 potentialIndex = treeWithFreeList->tree;
    MMNode *potential = treeNode;
    while (potentialIndex) {
        if (potential->data.memory.bytes >= bytes &&
            potential->data.memory.bytes <= bestSoFar) {
            bestSoFar = potential->data.memory.bytes;
            bestWithVisitedNodesLen = len;
        }

        visitedNodes[len].index = potentialIndex;

        U32 leftChildIndex =
            childNodePointerGet(&potential->header, RB_TREE_LEFT);
        MMNode *leftChild = getMMNode(treeWithFreeList, leftChildIndex);
        U32 rightChildIndex =
            childNodePointerGet(&potential->header, RB_TREE_RIGHT);
        MMNode *rightChild = getMMNode(treeWithFreeList, rightChildIndex);

        if (leftChild && leftChild->data.mostBytesInSubtree >= bytes) {
            if (rightChild && rightChild->data.mostBytesInSubtree >= bytes &&
                rightChild->data.mostBytesInSubtree <
                    leftChild->data.mostBytesInSubtree) {
                visitedNodes[len].direction = RB_TREE_RIGHT;
                potentialIndex = rightChildIndex;
                potential = rightChild;
            } else {
                visitedNodes[len].direction = RB_TREE_LEFT;
                potentialIndex = leftChildIndex;
                potential = leftChild;
            }
        } else {
            if (rightChild && rightChild->data.mostBytesInSubtree >= bytes) {
                visitedNodes[len].direction = RB_TREE_RIGHT;
                potentialIndex = rightChildIndex;
                potential = rightChild;
            } else {
                break;
            }
        }

        len++;
    }

    U32 deletedNodeIndex = deleteNodeInPath(
        treeWithFreeList, visitedNodes, bestWithVisitedNodesLen,
        visitedNodes[bestWithVisitedNodesLen].index);
    return deletedNodeIndex;
}
