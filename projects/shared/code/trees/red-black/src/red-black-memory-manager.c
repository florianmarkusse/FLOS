#include "shared/trees/red-black-memory-manager.h"
#include "shared/maths/maths.h"
#include "shared/memory/allocator/macros.h"
#include "shared/types/array.h"

typedef struct {
    RedBlackNodeMM *node;
    RedBlackDirection direction;
} VisitedNode;

static RedBlackDirection calculateDirection(U64 value,
                                            RedBlackNodeMM *toCompare) {
    if (value >= toCompare->memory.start) {
        return RB_TREE_RIGHT;
    }
    return RB_TREE_LEFT;
}

static void recalculateMostBytes(RedBlackNodeMM *node) {
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

static U64 rebalanceInsert(RedBlackDirection direction,
                           VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT],
                           U64 len) {
    RedBlackNodeMM *grandParent = visitedNodes[len - 3].node;
    RedBlackNodeMM *parent = visitedNodes[len - 2].node;
    RedBlackNodeMM *node = visitedNodes[len - 1].node;

    RedBlackNodeMM *uncle = grandParent->children[!direction];
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
        parent->children[!direction] = node->children[direction];
        node->children[direction] = parent;
        grandParent->children[direction] = node;

        node->mostBytesInSubtree = parent->mostBytesInSubtree;
        recalculateMostBytes(parent);

        node = node->children[direction];
        parent = grandParent->children[direction];
    }

    //      x           y
    //     /           / \
    //    y      ==>  z   x
    //   /
    //  z
    grandParent->children[direction] = parent->children[!direction];
    parent->children[!direction] = grandParent;
    visitedNodes[len - 4].node->children[visitedNodes[len - 4].direction] =
        parent; // NOTE: Can also be that we are setting the new
                // root pointer here!

    parent->color = RB_TREE_BLACK;
    parent->mostBytesInSubtree = grandParent->mostBytesInSubtree;

    grandParent->color = RB_TREE_RED;
    recalculateMostBytes(grandParent);

    return 0;
}

void insertRedBlackNodeMM(RedBlackNodeMM **tree, RedBlackNodeMM *createdNode) {
    createdNode->children[RB_TREE_LEFT] = nullptr;
    createdNode->children[RB_TREE_RIGHT] = nullptr;
    createdNode->mostBytesInSubtree = createdNode->memory.bytes;

    if (!(*tree)) {
        createdNode->color = RB_TREE_BLACK;
        *tree = createdNode;
        return;
    }

    // Search
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeMM *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    RedBlackNodeMM *current = *tree;
    while (1) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction =
            calculateDirection(createdNode->memory.start, current);
        current->mostBytesInSubtree =
            MAX(current->mostBytesInSubtree, createdNode->memory.bytes);
        len++;

        RedBlackNodeMM *next =
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
        len =
            rebalanceInsert(visitedNodes[len - 3].direction, visitedNodes, len);
    }

    (*tree)->color = RB_TREE_BLACK;
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
    RedBlackNodeMM *node = visitedNodes[len - 1].node;
    RedBlackNodeMM *childOtherDirection = node->children[!direction];
    // Ensure the other child is colored black, we "push" the problem a level
    // down in the process.
    //      x(B)              y(B)
    //       \               / \
    //        y(R)   ==>   x(R)a(B)
    //       / \            \
    //     z(B)a(B)        a(B)
    if (childOtherDirection->color == RB_TREE_RED) {
        childOtherDirection->color = RB_TREE_BLACK;
        node->color = RB_TREE_RED;

        node->children[!direction] = childOtherDirection->children[direction];
        childOtherDirection->children[direction] = node;
        visitedNodes[len - 2].node->children[visitedNodes[len - 2].direction] =
            childOtherDirection;

        childOtherDirection->mostBytesInSubtree = node->mostBytesInSubtree;
        recalculateMostBytes(node);

        visitedNodes[len - 1].node = childOtherDirection;
        visitedNodes[len].node = node;
        visitedNodes[len].direction = direction;
        len++;

        childOtherDirection = node->children[!direction];
    }

    RedBlackNodeMM *innerChildOtherDirection =
        childOtherDirection->children[direction];
    RedBlackNodeMM *outerChildOtherDirection =
        childOtherDirection->children[!direction];
    // Bubble up the problem by 1 level.
    if (((!innerChildOtherDirection) ||
         innerChildOtherDirection->color == RB_TREE_BLACK) &&
        ((!outerChildOtherDirection) ||
         outerChildOtherDirection->color == RB_TREE_BLACK)) {
        childOtherDirection->color = RB_TREE_RED;

        return len - 1;
    }

    //      x                     x
    //       \                     \
    //        y(B)          ===>   z(B)
    //       /                       \
    //     z(R)                      y(R)
    if ((!outerChildOtherDirection) ||
        outerChildOtherDirection->color == RB_TREE_BLACK) {
        childOtherDirection->color = RB_TREE_RED;
        innerChildOtherDirection->color = RB_TREE_BLACK;

        childOtherDirection->children[direction] =
            innerChildOtherDirection->children[!direction];
        innerChildOtherDirection->children[!direction] = childOtherDirection;
        node->children[!direction] = innerChildOtherDirection;

        RedBlackNodeMM *temp = childOtherDirection;
        childOtherDirection = innerChildOtherDirection;
        outerChildOtherDirection = temp;
    }

    //      x                          y
    //     / \                        / \
    //   a(B)y(B)        ===>       x(B)z(B)
    //         \                    /
    //        z(R)                a(B)
    childOtherDirection->color = node->color;
    node->color = RB_TREE_BLACK;
    outerChildOtherDirection->color = RB_TREE_BLACK;

    node->children[!direction] = childOtherDirection->children[direction];
    childOtherDirection->children[direction] = node;

    childOtherDirection->mostBytesInSubtree = node->mostBytesInSubtree;
    recalculateMostBytes(node);

    visitedNodes[len - 2].node->children[visitedNodes[len - 2].direction] =
        childOtherDirection;

    return 0;
}

static RedBlackNodeMM *
deleteNodeInPath(VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT], U64 len,
                 RedBlackNodeMM *toDelete) {
    // If there is no right child, we can delete by having the parent of
    // toDelete now point to toDelete's left child instead of toDelete.
    if (!(toDelete->children[RB_TREE_RIGHT])) {
        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            toDelete->children[RB_TREE_LEFT];
        recalculateMostBytes(visitedNodes[len - 1].node);
    }
    // Find the sucessor node in the subtree of toDelete's left chld. Done by
    // repeetedly going to the left child.
    else {
        visitedNodes[len].node = toDelete;
        visitedNodes[len].direction = RB_TREE_RIGHT;
        U64 foundNodeIndex = len;
        toDelete = toDelete->children[visitedNodes[len].direction];
        len++;

        while (true) {
            RedBlackNodeMM *next = toDelete->children[RB_TREE_LEFT];
            if (!next) {
                break;
            }

            visitedNodes[len].node = toDelete;
            visitedNodes[len].direction = RB_TREE_LEFT;
            len++;

            toDelete = next;
        }

        // Swap the values around. Naturally, the node pointers can be swapped
        // too.
        Memory foundMemory = visitedNodes[foundNodeIndex].node->memory;

        visitedNodes[foundNodeIndex].node->memory = toDelete->memory;
        // NOTE: Keep most bytes in subtree. The "higher" node we are swapping
        // has a right subtree so no changes to most bytes in subtree.

        toDelete->memory = foundMemory;

        visitedNodes[len - 1].node->children[visitedNodes[len - 1].direction] =
            toDelete->children[RB_TREE_RIGHT];
    }

    // Fix the violations present by removing the toDelete node. Note that this
    // node does not have to be the node that originally contained the value to
    // be deleted.
    if (toDelete->color == RB_TREE_BLACK) {
        while (len >= 2) {
            RedBlackNodeMM *childDeficitBlackDirection =
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

RedBlackNodeMM *deleteAtLeastRedBlackNodeMM(RedBlackNodeMM **tree, U64 bytes) {
    VisitedNode visitedNodes[RB_TREE_MAX_HEIGHT];

    visitedNodes[0].node = (RedBlackNodeMM *)tree;
    visitedNodes[0].direction = RB_TREE_LEFT;
    U64 len = 1;

    U64 bestWithVisitedNodesLen = 0;

    for (RedBlackNodeMM *potential = *tree; potential;) {
        if (potential->memory.bytes == bytes) {
            return deleteNodeInPath(visitedNodes, len, potential);
        }

        if (potential->memory.bytes > bytes) {
            bestWithVisitedNodesLen = len;
        }

        RedBlackDirection dir = calculateDirection(bytes, potential);
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
