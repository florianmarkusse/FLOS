#include "shared/trees/red-black/memory-manager.h"
#include "shared/maths.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/management/page.h"
#include "shared/memory/sizes.h"
#include "shared/trees/red-black/basic.h"
#include "shared/types/array.h"

typedef struct {
    MMNode *node;
    RedBlackDirection direction;
} MMVisitedNode;

static void recalculateMostBytes(MMNode *node) {
    node->mostBytesInSubtree = node->memory.bytes;
    if (node->children[RB_TREE_RIGHT]) {
        node->mostBytesInSubtree =
            MAX(node->mostBytesInSubtree,
                node->children[RB_TREE_RIGHT]->mostBytesInSubtree);
    }
    if (node->children[RB_TREE_LEFT]) {
        node->mostBytesInSubtree =
            MAX(node->mostBytesInSubtree,
                node->children[RB_TREE_LEFT]->mostBytesInSubtree);
    }
}

// The fist entry always contains the pointer to the address of the root node.
static constexpr auto ROOT_NODE_ADDRESS_LEN = 1;

static void
propagateInsertUpwards(U64 newMostBytesInSubtree,
                       MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                       U32 len) {
    while (len > ROOT_NODE_ADDRESS_LEN) {
        MMNode *node = visitedNodes[len - 1].node;
        if (node->mostBytesInSubtree >= newMostBytesInSubtree) {
            return;
        }
        node->mostBytesInSubtree = newMostBytesInSubtree;
        len--;
    }
}

static void
propagateDeleteUpwards(U64 deletedBytes,
                       MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len,
                       U32 end) {
    while (len > end) {
        MMNode *node = visitedNodes[len - 1].node;
        recalculateMostBytes(node);
        if (node->mostBytesInSubtree >= deletedBytes) {
            return;
        }
        len--;
    }
}

static void setMostBytesAfterRotation(void *prevRotationNode,
                                      void *prevRotationChild) {
    MMNode *previousRotationNode = prevRotationNode;
    MMNode *previousRotationChild = prevRotationChild;
    previousRotationChild->mostBytesInSubtree =
        previousRotationNode->mostBytesInSubtree;
    recalculateMostBytes(previousRotationNode);
}

static MMNode *deleteNodeInPath(MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                U32 len, MMNode *toDelete) {
    U32 stepsToSuccessor = findAdjacentInSteps(
        (RedBlackNode *)toDelete, (CommonVisitedNode *)&visitedNodes[len],
        RB_TREE_RIGHT);
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!stepsToSuccessor) {
        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            toDelete->children[RB_TREE_LEFT];
        propagateDeleteUpwards(toDelete->memory.bytes, visitedNodes, len,
                               ROOT_NODE_ADDRESS_LEN);
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
        Memory memoryToKeep = toDelete->memory;

        toDelete->memory = visitedNodes[upperNodeIndex - 1].node->memory;
        visitedNodes[upperNodeIndex - 1].node->memory = memoryToKeep;

        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            toDelete->children[RB_TREE_RIGHT];

        // In the first part, memoryToKeep got "deleted", i.e., moved higher in
        // the subtree. When we reach the node where the memoryTokeep is now at,
        // toDelete->memory got deleted.
        propagateDeleteUpwards(memoryToKeep.bytes, visitedNodes, len,
                               upperNodeIndex);
        propagateDeleteUpwards(toDelete->memory.bytes, visitedNodes,
                               upperNodeIndex, ROOT_NODE_ADDRESS_LEN);
    }

    // Fix the violations present by removing the toDelete node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (toDelete->color == RB_TREE_BLACK) {
        while (len >= 2) {
            MMNode *childDeficitBlackDirection =
                visitedNodes[len - 1]
                    .node->children[visitedNodes[len - 1].direction];
            if (childDeficitBlackDirection &&
                childDeficitBlackDirection->color == RB_TREE_RED) {
                childDeficitBlackDirection->color = RB_TREE_BLACK;
                break;
            }

            len = rebalanceDelete(visitedNodes[len - 1].direction,
                                  (VisitedNode *)visitedNodes, len,
                                  setMostBytesAfterRotation);
        }
    }

    return toDelete;
}

static void propogateInsertIfRequired(
    MMNode *current, MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U32 len) {
    if (current->memory.bytes > current->mostBytesInSubtree) {
        current->mostBytesInSubtree = current->memory.bytes;
        propagateInsertUpwards(current->memory.bytes, visitedNodes, len);
    }
}

static MMNode *bridgeMerge(MMNode *current, U64 adjacentBytes,
                           U32 adjacentSteps,
                           MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                           U32 len) {
    current->memory.bytes += adjacentBytes;
    propogateInsertIfRequired(current, visitedNodes, len);
    U32 newLen = len + adjacentSteps;
    return deleteNodeInPath(
        visitedNodes, newLen,
        visitedNodes[newLen - 1]
            .node->children[visitedNodes[newLen - 1].direction]);
}

static MMNode *beforeRegionMerge(MMNode *current, MMNode *createdNode,
                                 MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                 U32 len) {
    current->memory.bytes += createdNode->memory.bytes;

    U32 predecessorSteps = findAdjacentInSteps(
        (RedBlackNode *)current, (CommonVisitedNode *)&visitedNodes[len],
        RB_TREE_LEFT);
    if (predecessorSteps) {
        MMNode *predecessor =
            visitedNodes[len + predecessorSteps - 1]
                .node
                ->children[visitedNodes[len + predecessorSteps - 1].direction];
        // | predecessor | created | current |
        if (predecessor->memory.start + predecessor->memory.bytes ==
            createdNode->memory.start) {
            current->memory.start = predecessor->memory.start;
            return bridgeMerge(current, predecessor->memory.bytes,
                               predecessorSteps, visitedNodes, len);
        }
    }

    current->memory.start = createdNode->memory.start;
    propogateInsertIfRequired(current, visitedNodes, len);
    return nullptr;
}

static MMNode *afterRegionMerge(MMNode *current, MMNode *createdNode,
                                U64 createdEnd,
                                MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                U32 len) {
    current->memory.bytes += createdNode->memory.bytes;

    U32 successorSteps = findAdjacentInSteps(
        (RedBlackNode *)current, (CommonVisitedNode *)&visitedNodes[len],
        RB_TREE_RIGHT);
    if (successorSteps) {
        MMNode *successor =
            visitedNodes[len + successorSteps - 1]
                .node
                ->children[visitedNodes[len + successorSteps - 1].direction];
        // | current | created | successor |
        if (createdEnd == successor->memory.start) {
            return bridgeMerge(current, successor->memory.bytes, successorSteps,
                               visitedNodes, len);
        }
    }

    propogateInsertIfRequired(current, visitedNodes, len);
    return nullptr;
}

InsertResult insertMMNode(MMNode **tree, MMNode *createdNode) {
    InsertResult result = {0};

    createdNode->children[RB_TREE_LEFT] = nullptr;
    createdNode->children[RB_TREE_RIGHT] = nullptr;

    if (!(*tree)) {
        createdNode->color = RB_TREE_BLACK;
        createdNode->mostBytesInSubtree = createdNode->memory.bytes;
        *tree = createdNode;
        return result;
    }

    // Search
    MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (MMNode *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U32 len = 1;

    MMNode *current = *tree;
    U64 createdEnd = createdNode->memory.start + createdNode->memory.bytes;
    while (1) {
        U64 currentEnd = current->memory.start + current->memory.bytes;
        // | created | current |
        if (createdEnd == current->memory.start) {
            result.freed[0] = createdNode;
            result.freed[1] =
                beforeRegionMerge(current, createdNode, visitedNodes, len);
            return result;
        }
        // | current | created |
        else if (currentEnd == createdNode->memory.start) {
            result.freed[0] = createdNode;
            result.freed[1] = afterRegionMerge(current, createdNode, createdEnd,
                                               visitedNodes, len);
            return result;
        }

        visitedNodes[len].node = current;
        visitedNodes[len].direction = calculateDirection(
            createdNode->memory.start, current->memory.start);
        len++;

        MMNode *next = current->children[visitedNodes[len - 1].direction];
        if (!next) {
            break;
        }
        current = next;
    }

    // Insert
    createdNode->color = RB_TREE_RED;
    createdNode->mostBytesInSubtree = createdNode->memory.bytes;
    current->children[visitedNodes[len - 1].direction] = createdNode;
    propagateInsertUpwards(createdNode->memory.bytes, visitedNodes, len);

    // NOTE: we should never be looking at [len - 1].direction!
    visitedNodes[len].node = createdNode;
    len++;

    // Check for violations
    while (len >= 4 && visitedNodes[len - 2].node->color == RB_TREE_RED) {
        len = rebalanceInsert(visitedNodes[len - 3].direction, visitedNodes,
                              len, setMostBytesAfterRotation);
    }

    (*tree)->color = RB_TREE_BLACK;

    return result;
}

MMNode *deleteAtLeastMMNode(MMNode **tree, U64 bytes) {
    if (!(*tree) || (*tree)->mostBytesInSubtree < bytes) {
        return nullptr;
    }

    MMVisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (MMNode *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U32 len = 1;

    U64 bestSoFar = (*tree)->mostBytesInSubtree;
    U32 bestWithVisitedNodesLen = 0;

    for (MMNode *potential = *tree; potential;) {
        if (potential->memory.bytes >= bytes &&
            potential->memory.bytes <= bestSoFar) {
            bestSoFar = potential->memory.bytes;
            bestWithVisitedNodesLen = len;
        }

        visitedNodes[len].node = potential;

        MMNode *leftChild = potential->children[RB_TREE_LEFT];
        MMNode *rightChild = potential->children[RB_TREE_RIGHT];

        if (leftChild && leftChild->mostBytesInSubtree >= bytes) {
            if (rightChild && rightChild->mostBytesInSubtree >= bytes &&
                rightChild->mostBytesInSubtree <
                    leftChild->mostBytesInSubtree) {
                visitedNodes[len].direction = RB_TREE_RIGHT;
            } else {
                visitedNodes[len].direction = RB_TREE_LEFT;
            }
        } else {
            if (rightChild && rightChild->mostBytesInSubtree >= bytes) {
                visitedNodes[len].direction = RB_TREE_RIGHT;
            } else {
                break;
            }
        }

        potential = potential->children[visitedNodes[len].direction];
        len++;
    }

    return deleteNodeInPath(visitedNodes, bestWithVisitedNodesLen,
                            visitedNodes[bestWithVisitedNodesLen].node);
}
