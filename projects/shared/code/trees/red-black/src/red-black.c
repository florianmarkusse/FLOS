#include "shared/trees/red-black.h"
#include "shared/memory/allocator/macros.h"
#include "shared/types/array.h"

static constexpr auto MAX_HEIGHT = 128;

typedef struct {
    RedBlackNode **node;
    RedBlackDirection direction;
} VisitedNode;

static RedBlackDirection calculateDirection(RedBlackNode *createdNode,
                                            RedBlackNode *toCompare) {
    if (createdNode->value >= toCompare->value) {
        return RIGHT_CHILD;
    }
    return LEFT_CHILD;
}

void insertRedBlackNode(RedBlackNode **tree, RedBlackNode *createdNode) {
    if (!(*tree)) {
        *tree = createdNode;
        return;
    }

    // Search
    U64 len = 0;
    VisitedNode visitedNodes[MAX_HEIGHT];
    RedBlackNode **current = tree;
    while (*current) {
        visitedNodes[len].node = current;
        visitedNodes[len].direction = calculateDirection(createdNode, *current);
        current = &(*current)->children[visitedNodes[len].direction];

        len++;
    }

    // Insert
    createdNode->color = RED;
    *current = createdNode;

    // Check for violations
    if (len <= 2 || (*visitedNodes[len - 1].node)->color == BLACK) {
        return;
    }
}
void deleteRedBlackNode(RedBlackNode *tree, RedBlackNode *node) {
    // do stuff
    //
    //
}
RedBlackNode *findRedBlackNodeLeastBiggestValue(RedBlackNode *tree, U64 value) {
    // do stuff
    //
    //
}
