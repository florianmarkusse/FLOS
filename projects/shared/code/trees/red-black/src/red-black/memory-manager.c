#include "shared/trees/red-black/memory-manager.h"
#include "shared/maths.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/sizes.h"
#include "shared/types/array.h"

typedef struct {
    MMNode *node;
    RedBlackDirection direction;
} VisitedNode;

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

static void propagateInsertUpwards(U64 newMostBytesInSubtree,
                                   VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                   U64 len) {
    while (len > ROOT_NODE_ADDRESS_LEN) {
        MMNode *node = visitedNodes[len - 1].node;
        if (node->mostBytesInSubtree >= newMostBytesInSubtree) {
            return;
        }
        node->mostBytesInSubtree = newMostBytesInSubtree;
        len--;
    }
}

static void propagateDeleteUpwards(U64 deletedBytes,
                                   VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                   U64 len, U64 end) {
    while (len > end) {
        MMNode *node = visitedNodes[len - 1].node;
        recalculateMostBytes(node);
        if (node->mostBytesInSubtree >= deletedBytes) {
            return;
        }
        len--;
    }
}

static void setMostBytesAfterRotation(MMNode *prevRotationNode,
                                      MMNode *prevRotationChild) {
    prevRotationChild->mostBytesInSubtree =
        prevRotationNode->mostBytesInSubtree;
    recalculateMostBytes(prevRotationNode);
}

static U64 rebalanceInsert(RedBlackDirection direction,
                           VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                           U64 len) {
    MMNode *grandParent = visitedNodes[len - 3].node;
    MMNode *parent = visitedNodes[len - 2].node;
    MMNode *node = visitedNodes[len - 1].node;

    MMNode *uncle = grandParent->children[!direction];
    if (uncle && uncle->color == RB_TREE_RED) {
        uncle->color = RB_TREE_BLACK;
        parent->color = RB_TREE_BLACK;
        grandParent->color = RB_TREE_RED;

        return len - 2;
    }

    //      x             x
    //     /             /
    //    y       ==>   z
    //     \           /
    //      z         y
    if (visitedNodes[len - 2].direction == !direction) {
        rotateAround((RedBlackNode *)grandParent, (RedBlackNode *)parent,
                     (RedBlackNode *)node, direction, direction);
        setMostBytesAfterRotation(parent, node);

        node = node->children[direction];
        parent = grandParent->children[direction];
    }

    //      x           y
    //     /           / \
    //    y      ==>  z   x
    //   /
    //  z
    parent->color = RB_TREE_BLACK;
    grandParent->color = RB_TREE_RED;

    rotateAround((RedBlackNode *)visitedNodes[len - 4].node,
                 (RedBlackNode *)grandParent, (RedBlackNode *)parent,
                 !direction, visitedNodes[len - 4].direction);
    setMostBytesAfterRotation(grandParent, parent);

    return 0;
}

// We have 2 subtrees hanbing from visitedNodes - 1, the subtree of direction
// and the subtree of !direction (other direction). the subtree of direction has
// 1 less black height than the other subtree. The potential tree above these
// subtrees is now also missing a black node. Fixing the deficiency by removing
// a black node from the other direction subtree means that we stil need to
// address that problem. On the other hand, coloring a node black in the
// direction subtree immediately solves the deficiency in the whole tree.
static U64 rebalanceDelete(RedBlackDirection direction,
                           VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                           U64 len) {
    MMNode *node = visitedNodes[len - 1].node;
    MMNode *childOtherDirection = node->children[!direction];
    // Ensure the other child is colored black, we "push" the problem a level
    // down in the process.
    //                       x(B)              y(B)
    //                        \               / \
    // LEFT_DIRECTION          y(R)   ==>   x(R)a(B)
    //                        / \            \
    //                      z(B)a(B)        z(B)
    //
    //                        x(B)            y(B)
    //                        /               / \
    // RIGHT_DIRECTION     y(R)       ==>   a(B)x(R)
    //                     / \                  /
    //                   a(B)z(B)             z(B)
    if (childOtherDirection->color == RB_TREE_RED) {
        childOtherDirection->color = RB_TREE_BLACK;
        node->color = RB_TREE_RED;

        rotateAround((RedBlackNode *)visitedNodes[len - 2].node,
                     (RedBlackNode *)node, (RedBlackNode *)childOtherDirection,
                     direction, visitedNodes[len - 2].direction);
        setMostBytesAfterRotation(node, childOtherDirection);

        visitedNodes[len - 1].node = childOtherDirection;
        visitedNodes[len].node = node;
        visitedNodes[len].direction = direction;
        len++;

        childOtherDirection = node->children[!direction];
    }

    MMNode *innerChildOtherDirection =
        childOtherDirection->children[direction];
    MMNode *outerChildOtherDirection =
        childOtherDirection->children[!direction];
    // Bubble up the problem by 1 level.
    if (((!innerChildOtherDirection) ||
         innerChildOtherDirection->color == RB_TREE_BLACK) &&
        ((!outerChildOtherDirection) ||
         outerChildOtherDirection->color == RB_TREE_BLACK)) {
        childOtherDirection->color = RB_TREE_RED;

        return len - 1;
    }

    //                      x                 x
    //                       \                 \
    // LEFT_DIRECTION         y(B)   ===>     z(B)
    //                       /                   \
    //                     z(R)                  y(R)
    //
    //                      x                     x
    //                     /                     /
    // RIGHT_DIRECTION   y(B)        ===>      z(B)
    //                     \                   /
    //                     z(R)             y(R)
    if ((!outerChildOtherDirection) ||
        outerChildOtherDirection->color == RB_TREE_BLACK) {
        childOtherDirection->color = RB_TREE_RED;
        innerChildOtherDirection->color = RB_TREE_BLACK;

        rotateAround((RedBlackNode *)node, (RedBlackNode *)childOtherDirection,
                     (RedBlackNode *)innerChildOtherDirection, !direction,
                     !direction);
        setMostBytesAfterRotation(childOtherDirection,
                                  innerChildOtherDirection);

        MMNode *temp = childOtherDirection;
        childOtherDirection = innerChildOtherDirection;
        outerChildOtherDirection = temp;
    }

    //                         x                          y
    //                        / \                        / \
    // LEFT_DIRECTION       a(B)y(B)        ===>       x(B)z(B)
    //                            \                    /
    //                           z(R)                a(B)
    //
    //                         x                          y
    //                        / \                        / \
    // RIGHT_DIRECTION      y(B)a(B)        ===>       z(B)x(B)
    //                      /                               \
    //                    z(R)                              a(B)
    childOtherDirection->color = node->color;
    node->color = RB_TREE_BLACK;
    outerChildOtherDirection->color = RB_TREE_BLACK;

    rotateAround((RedBlackNode *)visitedNodes[len - 2].node,
                 (RedBlackNode *)node, (RedBlackNode *)childOtherDirection,
                 direction, visitedNodes[len - 2].direction);
    setMostBytesAfterRotation(node, childOtherDirection);

    return 0;
}

static MMNode *
deleteNodeInPath(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U64 len,
                 MMNode *toDelete) {
    U64 stepsToSuccessor = findAdjacentInSteps(
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
        U64 upperNodeIndex = len + 1;
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

            len = rebalanceDelete(visitedNodes[len - 1].direction, visitedNodes,
                                  len);
        }
    }

    return toDelete;
}

static void
propogateInsertIfRequired(MMNode *current,
                          VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                          U64 len) {
    if (current->memory.bytes > current->mostBytesInSubtree) {
        current->mostBytesInSubtree = current->memory.bytes;
        propagateInsertUpwards(current->memory.bytes, visitedNodes, len);
    }
}

static MMNode *bridgeMerge(MMNode *current, U64 adjacentBytes,
                                   U64 adjacentSteps,
                                   VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                                   U64 len) {
    current->memory.bytes += adjacentBytes;
    propogateInsertIfRequired(current, visitedNodes, len);
    U64 newLen = len + adjacentSteps;
    return deleteNodeInPath(
        visitedNodes, newLen,
        visitedNodes[newLen - 1]
            .node->children[visitedNodes[newLen - 1].direction]);
}

static MMNode *
beforeRegionMerge(MMNode *current, MMNode *createdNode,
                  VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U64 len) {
    current->memory.bytes += createdNode->memory.bytes;

    U64 predecessorSteps = findAdjacentInSteps(
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

static MMNode *
afterRegionMerge(MMNode *current, MMNode *createdNode,
                 U64 createdEnd, VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                 U64 len) {
    current->memory.bytes += createdNode->memory.bytes;

    U64 successorSteps = findAdjacentInSteps(
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

InsertResult insertMMNode(MMNode **tree,
                                  MMNode *createdNode) {
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
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (MMNode *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

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

        MMNode *next =
            current->children[visitedNodes[len - 1].direction];
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
        len =
            rebalanceInsert(visitedNodes[len - 3].direction, visitedNodes, len);
    }

    (*tree)->color = RB_TREE_BLACK;

    return result;
}

MMNode *deleteAtLeastMMNode(MMNode **tree, U64 bytes) {
    if (!(*tree) || (*tree)->mostBytesInSubtree < bytes) {
        return nullptr;
    }

    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (MMNode *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    U64 bestSoFar = (*tree)->mostBytesInSubtree;
    U64 bestWithVisitedNodesLen = 0;

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
