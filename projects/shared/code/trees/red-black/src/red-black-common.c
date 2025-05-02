#include "shared/trees/red-black-common.h"

VisitedRedBlackNode_a successor(RedBlackNode *node,
                                VisitedRedBlackNode_a visited) {
    if (!node->children[RB_TREE_RIGHT]) {
        return visited;
    }

    visited.buf[visited.len].node = node;
    visited.buf[visited.len].direction = RB_TREE_RIGHT;
    node = node->children[RB_TREE_RIGHT];
    visited.len++;

    while (true) {
        RedBlackNode *next = node->children[RB_TREE_LEFT];
        if (!next) {
            break;
        }

        visited.buf[visited.len].node = node;
        visited.buf[visited.len].direction = RB_TREE_LEFT;
        visited.len++;

        node = next;
    }

    return visited;
}
