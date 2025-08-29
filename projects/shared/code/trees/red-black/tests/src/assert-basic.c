#include "shared/trees/red-black/tests/assert-basic.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/trees/red-black/tests/assert.h"
#include "shared/trees/red-black/virtual-mapping-manager.h"
#include "shared/types/array-types.h"

typedef struct {
    U64 value;
    U32 index;
} NodeIndexMemory;

typedef ARRAY(NodeIndexMemory) NodeIndexMemory_a;

static void inOrderTraversalFillValues(NodeLocation *nodeLocation, U32 node,
                                       NodeIndexMemory_a *values) {
    if (!node) {
        return;
    }

    RedBlackNode *treeNode = getNode(nodeLocation, node);

    inOrderTraversalFillValues(
        nodeLocation, childNodePointerGet(treeNode, RB_TREE_LEFT), values);
    values->buf[values->len] = (NodeIndexMemory){
        .index = node,
        .value = getVMMNode(nodeLocation, node)->data.memory.start};
    values->len++;
    inOrderTraversalFillValues(
        nodeLocation, childNodePointerGet(treeNode, RB_TREE_RIGHT), values);
}

static void appendExpectedValues(U64_max_a expectedValues) {
    INFO(STRING("Expected values:\n"));
    for (U32 i = 0; i < expectedValues.len; i++) {
        INFO(expectedValues.buf[i]);
        INFO(STRING(" "));
    }
}

static void appendExpectedValuesAndTreeValues(U64_max_a expectedValues,
                                              NodeIndexMemory_a inOrderValues) {
    appendExpectedValues(expectedValues);
    INFO(STRING("\nRed-Black Tree values:\n"));
    for (U32 i = 0; i < inOrderValues.len; i++) {
        INFO(inOrderValues.buf[i].value);
        INFO(STRING(" "));
    }
    INFO(STRING("\n"));
}

static void assertIsBSTWitExpectedValues(NodeLocation *nodeLocation, U32 node,
                                         U32 nodes, U64_max_a expectedValues,
                                         Arena scratch) {
    NodeIndexMemory_a inOrderValues = {
        .buf = NEW(&scratch, NodeIndexMemory, .count = nodes), .len = 0};

    inOrderTraversalFillValues(nodeLocation, node, &inOrderValues);

    if (inOrderValues.len != expectedValues.len) {
        TEST_FAILURE {
            INFO(STRING("The Red-Black Tree does not contain all the values it "
                        "should contain or it contains more!\n"));
            appendExpectedValuesAndTreeValues(expectedValues, inOrderValues);
            appendRedBlackTreeWithBadNode(nodeLocation, node, 0,
                                          RED_BLACK_VIRTUAL_MEMORY_MAPPER);
        }
    }

    for (U32 i = 0; i < expectedValues.len; i++) {
        bool found = false;
        for (U32 j = 0; j < inOrderValues.len; j++) {
            if (inOrderValues.buf[j].value == expectedValues.buf[i]) {
                found = true;
                break;
            }
        }

        if (!found) {
            TEST_FAILURE {
                INFO(STRING("The Red-Black Tree does not contain the value "));
                INFO(expectedValues.buf[i], .flags = NEWLINE);
                appendExpectedValuesAndTreeValues(expectedValues,
                                                  inOrderValues);
                appendRedBlackTreeWithBadNode(nodeLocation, node, 0,
                                              RED_BLACK_VIRTUAL_MEMORY_MAPPER);
            }
        }
    }

    U64 previous = 0;
    for (U32 i = 0; i < inOrderValues.len; i++) {
        if (previous > inOrderValues.buf[i].value) {
            TEST_FAILURE {
                INFO(STRING("Not a Binary Search Tree!\n"));
                appendRedBlackTreeWithBadNode(nodeLocation, node,
                                              inOrderValues.buf[i].index,
                                              RED_BLACK_VIRTUAL_MEMORY_MAPPER);
            }
        }
        previous = inOrderValues.buf[i].value;
    }
}

void assertBasicRedBlackTreeValid(NodeLocation *nodeLocation, U32 tree,
                                  U64_max_a expectedValues, Arena scratch) {
    if (!tree) {
        if (expectedValues.len == 0) {
            return;
        }
        TEST_FAILURE {
            INFO(STRING(
                "The Red-Black Tree is empty while expected values is not!\n"));
            appendExpectedValues(expectedValues);
            INFO(STRING("\n"));
        }
    }

    U32 nodes = nodeCount(nodeLocation, tree, RED_BLACK_VIRTUAL_MEMORY_MAPPER);

    assertIsBSTWitExpectedValues(nodeLocation, tree, nodes, expectedValues,
                                 scratch);
    assertNoRedNodeHasRedChild(nodeLocation, tree, nodes,
                               RED_BLACK_VIRTUAL_MEMORY_MAPPER, scratch);
    assertPathsFromNodeHaveSameBlackHeight(
        nodeLocation, tree, nodes, RED_BLACK_VIRTUAL_MEMORY_MAPPER, scratch);
}
