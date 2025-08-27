#include "shared/trees/red-black/memory-manager.h"
#include "shared/maths.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/basic.h"
#include "shared/types/array.h"

MMNode *getMMNode(NodeLocation *nodeLocation, U32 index) {
    return (MMNode *)getNode(nodeLocation, index);
}

static void initVisitedNodes(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                             U32 *len) {
    visitedNodes[0].index = 0;
    // NOTE: we should never be looking at [0].direction!
    *len = 1;
}

static void recalculateMostBytes(NodeLocation *nodeLocation, MMNode *node) {
    node->data.mostBytesInSubtree = node->data.memory.bytes;
    if (node->header.children[RB_TREE_RIGHT]) {
        MMNode *childNode =
            getMMNode(nodeLocation, node->header.children[RB_TREE_RIGHT]);
        node->data.mostBytesInSubtree = MAX(node->data.mostBytesInSubtree,
                                            childNode->data.mostBytesInSubtree);
    }
    if (node->header.children[RB_TREE_LEFT]) {
        MMNode *childNode =
            getMMNode(nodeLocation, node->header.children[RB_TREE_LEFT]);
        node->data.mostBytesInSubtree = MAX(node->data.mostBytesInSubtree,
                                            childNode->data.mostBytesInSubtree);
    }
}

// The fist entry always contains the pointer to the address of the root node.
static constexpr auto ROOT_NODE_INDEX_LEN = 1;

static void propagateInsertUpwards(NodeLocation *nodeLocation,
                                   VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                   U32 len, U64 newMostBytesInSubtree) {
    while (len > ROOT_NODE_INDEX_LEN) {
        MMNode *node = getMMNode(nodeLocation, visitedNodes[len - 1].index);
        if (node->data.mostBytesInSubtree >= newMostBytesInSubtree) {
            return;
        }
        node->data.mostBytesInSubtree = newMostBytesInSubtree;
        len--;
    }
}

static void propagateDeleteUpwards(NodeLocation *nodeLocation,
                                   VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                   U32 len, U64 deletedBytes, U32 end) {
    while (len > end) {
        MMNode *node = getMMNode(nodeLocation, visitedNodes[len - 1].index);
        recalculateMostBytes(nodeLocation, node);
        if (node->data.mostBytesInSubtree >= deletedBytes) {
            return;
        }
        len--;
    }
}

static void setMostBytesAfterRotation(NodeLocation *nodeLocation,
                                      U32 prevRotationNode,
                                      U32 prevRotationChild) {
    MMNode *previousRotationNode = getMMNode(nodeLocation, prevRotationNode);
    MMNode *previousRotationChild = getMMNode(nodeLocation, prevRotationChild);
    previousRotationChild->data.mostBytesInSubtree =
        previousRotationNode->data.mostBytesInSubtree;
    recalculateMostBytes(nodeLocation, previousRotationNode);
}

static U32 deleteNodeInPath(NodeLocation *nodeLocation,
                            VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                            U32 len, U32 *tree, U32 toDelete) {
    U32 stepsToSuccessor = findAdjacentInSteps(nodeLocation, &visitedNodes[len],
                                               toDelete, RB_TREE_RIGHT);

    MMNode *toDeleteNode = getMMNode(nodeLocation, toDelete);
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!stepsToSuccessor) {
        getMMNode(nodeLocation, visitedNodes[len - 1].index)
            ->header.children[visitedNodes[len - 1].direction] =
            getMMNode(nodeLocation, visitedNodes[len - 1].index)
                ->header.children[RB_TREE_LEFT];
        propagateDeleteUpwards(nodeLocation, visitedNodes, len,
                               toDeleteNode->data.memory.bytes,
                               ROOT_NODE_INDEX_LEN);
    }
    // Swap the values of the node to delete with the values of the successor
    // node and delete the successor node instead (now containing the values
    // of the to delete node).
    else {
        U32 upperNodeIndex = len + 1;
        len += stepsToSuccessor;

        MMNode *nodeGettingNewChild =
            getMMNode(nodeLocation, visitedNodes[len - 1].index);
        toDelete = nodeGettingNewChild->header
                       .children[visitedNodes[len - 1].direction];

        // Swap the values around. Naturally, the node pointers can be
        // swapped too. Just swapping memory here, mostBytesInSubtree will be
        // updated after.
        toDeleteNode = getMMNode(nodeLocation, toDelete);
        MMNode *swapNode =
            getMMNode(nodeLocation, visitedNodes[upperNodeIndex - 1].index);

        Memory memoryToKeep = toDeleteNode->data.memory;

        toDeleteNode->data.memory = swapNode->data.memory;
        swapNode->data.memory = memoryToKeep;

        nodeGettingNewChild->header.children[visitedNodes[len - 1].direction] =
            toDeleteNode->header.children[RB_TREE_RIGHT];

        // In the first part, memoryToKeep got "deleted", i.e., moved higher
        // in
        // the subtree. When we reach the node where the memoryTokeep is now
        //   at,
        // toDelete->memory got deleted.
        propagateDeleteUpwards(nodeLocation, visitedNodes, len,
                               memoryToKeep.bytes, upperNodeIndex);
        propagateDeleteUpwards(nodeLocation, visitedNodes, upperNodeIndex,
                               toDeleteNode->data.memory.bytes,
                               ROOT_NODE_INDEX_LEN);
    }

    // Fix the violations present by removing the toDelete node. Note that
    // this node does not have to be the node that originally contained the
    // value to be deleted.
    if (getColorWithPointer(&toDeleteNode->header) == RB_TREE_BLACK) {
        while (len >= 2) {
            U32 childDeficitBlackDirection =
                getMMNode(nodeLocation, visitedNodes[len - 1].index)
                    ->header.children[visitedNodes[len - 1].direction];
            MMNode *childDeficitBlackDirectionNode =
                getMMNode(nodeLocation, childDeficitBlackDirection);
            if (childDeficitBlackDirection &&
                getColorWithPointer(&childDeficitBlackDirectionNode->header) ==
                    RB_TREE_RED) {
                setColorWithPointer(&childDeficitBlackDirectionNode->header,
                                    RB_TREE_BLACK);
                break;
            }

            len = rebalanceDelete(nodeLocation, visitedNodes, len, tree,
                                  visitedNodes[len - 1].direction,
                                  setMostBytesAfterRotation);
        }
    }

    return toDelete;
}

static void
propogateInsertIfRequired(NodeLocation *nodeLocation,
                          VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                          MMNode *current) {
    if (current->data.memory.bytes > current->data.mostBytesInSubtree) {
        current->data.mostBytesInSubtree = current->data.memory.bytes;
        propagateInsertUpwards(nodeLocation, visitedNodes, len,
                               current->data.memory.bytes);
    }
}

static U32 bridgeMerge(NodeLocation *nodeLocation,
                       VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                       U32 *tree, MMNode *current, U64 adjacentBytes,
                       U32 adjacentSteps) {
    current->data.memory.bytes += adjacentBytes;
    propogateInsertIfRequired(nodeLocation, visitedNodes, len, current);
    U32 newLen = len + adjacentSteps;

    U32 previousNewLen = newLen - 1;
    MMNode *previousNewParent =
        getMMNode(nodeLocation, visitedNodes[previousNewLen].index);

    return deleteNodeInPath(
        nodeLocation, visitedNodes, newLen, tree,
        previousNewParent->header
            .children[visitedNodes[previousNewLen].direction]);
}

static U32 beforeRegionMerge(NodeLocation *nodeLocation,
                             VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                             U32 len, U32 *tree, U32 current,
                             MMNode *createdNode) {
    MMNode *currentNode = getMMNode(nodeLocation, current);
    currentNode->data.memory.bytes += createdNode->data.memory.bytes;

    U32 predecessorSteps = findAdjacentInSteps(nodeLocation, &visitedNodes[len],
                                               current, RB_TREE_LEFT);
    if (predecessorSteps) {
        U32 predecessorLen = len + predecessorSteps - 1;
        MMNode *predecessorParent =
            getMMNode(nodeLocation, visitedNodes[predecessorLen].index);
        MMNode *predecessor =
            getMMNode(nodeLocation,
                      predecessorParent->header
                          .children[visitedNodes[predecessorLen].direction]);
        // | predecessor | created | current |
        if (predecessor->data.memory.start + predecessor->data.memory.bytes ==
            createdNode->data.memory.start) {
            currentNode->data.memory.start = predecessor->data.memory.start;
            return bridgeMerge(nodeLocation, visitedNodes, len, tree,
                               currentNode, predecessor->data.memory.bytes,
                               predecessorSteps);
        }
    }

    currentNode->data.memory.start = createdNode->data.memory.start;
    propogateInsertIfRequired(nodeLocation, visitedNodes, len, currentNode);
    return 0;
}

static U32 afterRegionMerge(NodeLocation *nodeLocation,
                            VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                            U32 len, U32 *tree, U32 current,
                            MMNode *createdNode, U64 createdEnd) {
    MMNode *currentNode = getMMNode(nodeLocation, current);
    currentNode->data.memory.bytes += createdNode->data.memory.bytes;

    U32 successorSteps = findAdjacentInSteps(nodeLocation, &visitedNodes[len],
                                             current, RB_TREE_RIGHT);
    if (successorSteps) {
        U32 successorLen = len + successorSteps - 1;
        MMNode *successorParent =
            getMMNode(nodeLocation, visitedNodes[successorLen].index);
        MMNode *successor = getMMNode(
            nodeLocation, successorParent->header
                              .children[visitedNodes[successorLen].direction]);
        // | current | created | successor |
        if (createdEnd == successor->data.memory.start) {
            return bridgeMerge(nodeLocation, visitedNodes, len, tree,
                               currentNode, successor->data.memory.bytes,
                               successorSteps);
        }
    }

    propogateInsertIfRequired(nodeLocation, visitedNodes, len, currentNode);

    return 0;
}

InsertResult insertMMNode(MMTreeWithFreeList *treeWithFreeList,
                          MMNode *createdNode) {
    NodeLocation *nodeLocation = &treeWithFreeList->nodeLocation;
    U32 *tree = treeWithFreeList->tree;

    InsertResult result = {.freed = {0}};

    createdNode->header.children[RB_TREE_LEFT] = 0;
    createdNode->header.children[RB_TREE_RIGHT] = 0;

    U32 createdNodeIndex = getIndex(nodeLocation, createdNode);
    if (!(*tree)) {
        // NOTE: Set created node to black, is already done by setting children
        // to 0
        createdNode->data.mostBytesInSubtree = createdNode->data.memory.bytes;
        *tree = createdNodeIndex;
        return result;
    }

    // Search
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U32 current = *tree;
    MMNode *currentNode = getMMNode(nodeLocation, current);
    U64 createdEnd =
        createdNode->data.memory.start + createdNode->data.memory.bytes;
    while (1) {
        U64 currentEnd =
            currentNode->data.memory.start + currentNode->data.memory.bytes;
        // | created | current |
        if (createdEnd == currentNode->data.memory.start) {
            result.freed[0] = getIndex(nodeLocation, createdNode);
            result.freed[1] = beforeRegionMerge(nodeLocation, visitedNodes, len,
                                                tree, current, createdNode);
            return result;
        }
        // | current | created |
        else if (currentEnd == createdNode->data.memory.start) {
            result.freed[0] = createdNodeIndex;
            result.freed[1] =
                afterRegionMerge(nodeLocation, visitedNodes, len, tree, current,
                                 createdNode, createdEnd);
            return result;
        }

        visitedNodes[len].index = current;
        visitedNodes[len].direction = calculateDirection(
            createdNode->data.memory.start, currentNode->data.memory.start);
        len++;

        U32 next =
            currentNode->header.children[visitedNodes[len - 1].direction];
        if (!next) {
            break;
        }

        current = next;
        currentNode = getMMNode(nodeLocation, current);
    }

    // Insert
    setColorWithPointer(&createdNode->header, RB_TREE_RED);
    createdNode->data.mostBytesInSubtree = createdNode->data.memory.bytes;
    currentNode->header.children[visitedNodes[len - 1].direction] =
        createdNodeIndex;
    propagateInsertUpwards(nodeLocation, visitedNodes, len,
                           createdNode->data.memory.bytes);

    // NOTE: we should never be looking at [len - 1].direction!
    visitedNodes[len].index = createdNodeIndex;
    len++;

    // Check for violations
    while (len >= 4 &&
           getColor(nodeLocation, visitedNodes[len - 2].index) == RB_TREE_RED) {
        len = rebalanceInsert(nodeLocation, visitedNodes, len, tree,
                              visitedNodes[len - 3].direction,
                              setMostBytesAfterRotation);
    }

    setColor(nodeLocation, *tree, RB_TREE_BLACK);

    return result;
}

U32 deleteAtLeastMMNode(MMTreeWithFreeList *treeWithFreeList, U64 bytes) {
    NodeLocation *nodeLocation = &treeWithFreeList->nodeLocation;
    U32 *tree = treeWithFreeList->tree;

    if (!*tree) {
        return 0;
    }

    MMNode *treeNode = getMMNode(nodeLocation, *tree);
    if (treeNode->data.mostBytesInSubtree < bytes) {
        return 0;
    }

    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];
    U32 len;
    initVisitedNodes(visitedNodes, &len);

    U64 bestSoFar = treeNode->data.mostBytesInSubtree;
    U32 bestWithVisitedNodesLen = 0;

    U32 potentialIndex = *tree;
    for (MMNode *potential = treeNode; potential;) {
        if (potential->data.memory.bytes >= bytes &&
            potential->data.memory.bytes <= bestSoFar) {
            bestSoFar = potential->data.memory.bytes;
            bestWithVisitedNodesLen = len;
        }

        visitedNodes[len].index = potentialIndex;

        U32 leftChildIndex = potential->header.children[RB_TREE_LEFT];
        MMNode *leftChild = getMMNode(nodeLocation, leftChildIndex);
        U32 rightChildIndex = potential->header.children[RB_TREE_RIGHT];
        MMNode *rightChild = getMMNode(nodeLocation, rightChildIndex);

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

    U32 deletedNodeIndex =
        deleteNodeInPath(nodeLocation, visitedNodes, bestWithVisitedNodesLen,
                         tree, visitedNodes[bestWithVisitedNodesLen].index);
    return deletedNodeIndex;
}
