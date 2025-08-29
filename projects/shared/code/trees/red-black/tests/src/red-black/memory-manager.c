#include "shared/trees/red-black/tests/red-black/memory-manager.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "posix/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/policy/status.h"
#include "shared/text/string.h"
#include "shared/trees/red-black/basic.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/trees/red-black/tests/assert-basic.h"
#include "shared/trees/red-black/tests/assert-memory-manager.h"
#include "shared/trees/red-black/tests/assert.h"
#include "shared/trees/red-black/tests/cases/memory-manager.h"
#include "shared/trees/red-black/tests/red-black/common.h"

static void addValueToExpected(NodeLocation *nodeLocation,
                               Memory_max_a *expectedValues, Memory toAdd,
                               U32 tree) {
    U64 indexToInsert = 0;
    while (indexToInsert < expectedValues->len &&
           expectedValues->buf[indexToInsert].start < toAdd.start) {
        indexToInsert++;
    }

    bool mergeBefore = false;
    if (indexToInsert > 0) {
        U64 endBefore = expectedValues->buf[indexToInsert - 1].start +
                        expectedValues->buf[indexToInsert - 1].bytes;
        mergeBefore = endBefore == toAdd.start;
    }

    bool mergeAfter = false;
    if (indexToInsert < expectedValues->len) {
        U64 endToAdd = toAdd.start + toAdd.bytes;
        mergeAfter = expectedValues->buf[indexToInsert].start == endToAdd;
    }

    if (mergeBefore && mergeAfter) {
        expectedValues->buf[indexToInsert - 1].bytes +=
            toAdd.bytes + expectedValues->buf[indexToInsert].bytes;

        U64 afterMergeIndex = indexToInsert + 1;
        if (afterMergeIndex < expectedValues->len) {
            memmove(&(expectedValues->buf[indexToInsert]),
                    &(expectedValues->buf[afterMergeIndex]),
                    (expectedValues->len - afterMergeIndex) * sizeof(toAdd));
        }
        expectedValues->len--;
    } else if (mergeBefore) {
        expectedValues->buf[indexToInsert - 1].bytes += toAdd.bytes;
    } else if (mergeAfter) {
        expectedValues->buf[indexToInsert].start -= toAdd.bytes;
        expectedValues->buf[indexToInsert].bytes += toAdd.bytes;
    } else {
        if (expectedValues->len >= MAX_NODES_IN_TREE) {
            TEST_FAILURE {
                INFO(STRING("Tree contains too many nodes to fit in array. "
                            "Increase max size or decrease expected nodes "
                            "in Red-Black tree. Current maximum size: "));
                INFO(MAX_NODES_IN_TREE, .flags = NEWLINE);
                appendRedBlackTreeWithBadNode(nodeLocation, tree, 0,
                                              RED_BLACK_MEMORY_MANAGER);
            }
        }

        memmove(&(expectedValues->buf[indexToInsert + 1]),
                &(expectedValues->buf[indexToInsert]),
                (expectedValues->len - indexToInsert) * sizeof(toAdd));

        expectedValues->buf[indexToInsert] = toAdd;
        expectedValues->len++;
    }
}

static void testTree(TreeOperation_a operations, Arena scratch) {
    U32 tree = 0;
    MMTreeWithFreeList treeWithFreeList = {
        .nodes = (MMNode_max_a){.buf = NEW(&scratch, MMNode,
                                           .count = MAX_NODES_IN_TREE),
                                .len = 0,
                                .cap = MAX_NODES_IN_TREE},
        .tree = &tree,
        .freeList =
            (U32_max_a){.buf = NEW(&scratch, U32, .count = MAX_NODES_IN_TREE),
                        .len = 0,
                        .cap = MAX_NODES_IN_TREE},
        .nodeLocation = {.base = (U8 *)treeWithFreeList.nodes.buf,
                         .elementSizeBytes =
                             sizeof(*treeWithFreeList.nodes.buf)}};
    treeWithFreeList.nodes.buf[0] = (MMNode){0};
    treeWithFreeList.nodes.len = 1;

    Memory_max_a expectedValues =
        (Memory_max_a){.buf = NEW(&scratch, Memory, .count = MAX_NODES_IN_TREE),
                       .len = 0,
                       .cap = MAX_NODES_IN_TREE};
    for (U64 i = 0; i < operations.len; i++) {
        switch (operations.buf[i].type) {
        case INSERT: {
            addValueToExpected(&treeWithFreeList.nodeLocation, &expectedValues,
                               operations.buf[i].memory, tree);

            MMNode *createdNode =
                getFromNodes((TreeWithFreeList *)&treeWithFreeList.nodes);
            createdNode->data.memory = operations.buf[i].memory;
            (void)insertMMNode(&treeWithFreeList, createdNode);
            break;
        }
        case DELETE_AT_LEAST: {
            U32 deletedIndex = deleteAtLeastMMNode(
                &treeWithFreeList, operations.buf[i].memory.bytes);
            if (!deletedIndex) {
                for (U64 j = 0; j < expectedValues.len; j++) {
                    if (expectedValues.buf[j].bytes >=
                        operations.buf[i].memory.bytes) {
                        TEST_FAILURE {
                            INFO(STRING("Did not find a node todelete value "));
                            INFO(operations.buf[i].memory.bytes);
                            INFO(STRING(
                                ", should have deleted node with value: "));
                            INFO(expectedValues.buf[i].bytes, .flags = NEWLINE);
                        }
                    }
                }
                break;
            } else {
                MMNode *deleted =
                    getMMNode(&treeWithFreeList.nodeLocation, deletedIndex);
                if (deleted->data.memory.bytes <
                    operations.buf[i].memory.bytes) {
                    TEST_FAILURE {
                        INFO(STRING(
                            "Deleted value not equal the value that should "
                            "have been deleted !\nExpected to be "
                            "deleted value: "));
                        INFO(operations.buf[i].memory.bytes, .flags = NEWLINE);
                        INFO(STRING("Actual deleted value: "));
                        INFO(deleted->data.memory.bytes, .flags = NEWLINE);
                    }
                } else {
                    U64 indexToRemove = 0;
                    for (U64 j = 0; j < expectedValues.len; j++) {
                        if (expectedValues.buf[j].start ==
                            deleted->data.memory.start) {
                            indexToRemove = j;
                        }
                    }
                    memmove(&(expectedValues.buf[indexToRemove]),
                            &(expectedValues.buf[indexToRemove + 1]),
                            (expectedValues.len - indexToRemove) *
                                sizeof(expectedValues.buf[0]));
                    expectedValues.len--;

                    break;
                }
            }
        }
        }

        assertMMRedBlackTreeValid(&treeWithFreeList.nodeLocation, tree,
                                  expectedValues, scratch);
    }

    testSuccess();
}

static void testSubTopic(String subTopic, TestCases testCases, Arena scratch) {
    TEST_TOPIC(subTopic) {
        JumpBuffer failureHandler;
        for (U64 i = 0; i < testCases.len; i++) {
            if (setjmp(failureHandler)) {
                continue;
            }
            TEST(U64ToStringDefault(i), failureHandler) {
                testTree(testCases.buf[i], scratch);
            }
        }
    }
}

void testMemoryManagerRedBlackTrees(Arena scratch) {
    TEST_TOPIC(STRING("Memory Manager red-black trees")) {
        testSubTopic(STRING("No Operations"), noOperationsTestCase, scratch);
        testSubTopic(STRING("Insert Only"), insertsOnlyTestCases, scratch);
        testSubTopic(STRING("Insert + At Least Deletions"),
                     insertDeleteAtLeastsOnlyTestCases, scratch);
    }
}
