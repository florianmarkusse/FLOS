#include "shared/trees/red-black.h"
#include "shared/memory/allocator/macros.h"

void simpleInsert(RedBlackNode **node, RedBlackNode *createdNode, bool dir) {
    if (!(*node)->children[dir]) {
        (*node)->children[dir] = createdNode;
        return;
    }

    simpleInsert(&(*node)->children[dir], createdNode, !dir);
}

void insertRedBlackNode(RedBlackNode **node, U64 value, Arena *perm) {
    RedBlackNode *createdNode = NEW(perm, RedBlackNode, 1, ZERO_MEMORY);
    createdNode->value = value;

    if (!(*node)) {
        *node = createdNode;
        return;
    }

    simpleInsert(node, createdNode, true);
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
