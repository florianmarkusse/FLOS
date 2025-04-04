#include "shared/trees/red-black/tests/correct.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/trees/red-black.h"
#include "shared/types/array-types.h"

static constexpr auto MAX_NODES_TEST_TREE = 1024;

void printTreeIndented(RedBlackNode *node, int depth, string prefix,
                       RedBlackNode *badNode) {
    if (!node)
        return;

    printTreeIndented(node->children[RIGHT_CHILD], depth + 1, STRING("R---"),
                      badNode);

    for (int i = 0; i < depth; i++) {
        INFO(STRING("    "));
    }
    INFO(prefix);
    INFO(STRING("Value: "));
    INFO(node->value);
    INFO(STRING(", Color: "));
    INFO(node->color == RED ? STRING("RED") : STRING("BLACK"));

    if (node == badNode) {
        appendColor(COLOR_RED, STDOUT);
        INFO(STRING("    !!!!    !!!!"));
        appendColorReset(STDOUT);
    }

    INFO(STRING("\n"));

    printTreeIndented(node->children[LEFT_CHILD], depth + 1, STRING("L---"),
                      badNode);
}

void printRedBlackTree(RedBlackNode *root, RedBlackNode *badNode) {
    INFO(STRING("Red-Black Tree Structure:"), NEWLINE);
    printTreeIndented(root, 0, STRING("Root---"), badNode);
}

typedef ARRAY(RedBlackNode *) RedBlackNodePtr_a;
void inOrderTraversal(RedBlackNode *node, RedBlackNodePtr_a values) {
    if (node->children[LEFT_CHILD]) {
        inOrderTraversal(node->children[LEFT_CHILD], values);
    }
    values.buf[values.len] = node;
    values.len++;
    if (node->children[RIGHT_CHILD]) {
        inOrderTraversal(node->children[RIGHT_CHILD], values);
    }
}

bool isBinarySearchTree(RedBlackNode *node, Arena scratch) {
    RedBlackNodePtr_a inOrderValues = (RedBlackNodePtr_a){
        .buf = NEW(&scratch, RedBlackNode *, MAX_NODES_TEST_TREE), .len = 0};
    inOrderTraversal(node, inOrderValues);

    U64 previous = 0;
    for (U64 i = 0; i < inOrderValues.len; i++) {
        if (previous > inOrderValues.buf[i]->value) {
            TEST_FAILURE {
                INFO(STRING(
                    "The Red-Black Tree is not a Binary Search Tree!\n"));
                printRedBlackTree(node, inOrderValues.buf[i]);
            }
            return false;
        }
        previous = inOrderValues.buf[i]->value;
    }

    KFLUSH_AFTER {
        INFO(STRING("The len is: "));
        INFO(inOrderValues.len, NEWLINE);
        for (U64 i = 0; i < inOrderValues.len; i++) {
            INFO(STRING("The value is: "));
            INFO(inOrderValues.buf[i]->value, NEWLINE);
        }
    }

    return true;
}

void assertRedBlackTreeValid(RedBlackNode *tree, Arena scratch) {
    if (tree == nullptr) {
        testSuccess();
        return;
    }

    if (!isBinarySearchTree(tree, scratch)) {
        return;
    }

    testSuccess();
}
