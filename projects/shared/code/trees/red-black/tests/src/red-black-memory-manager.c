#include "shared/trees/red-black/tests/red-black-memory-manager.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/memory/allocator/macros.h"
#include "shared/text/string.h"
#include "shared/trees/red-black-basic.h"
#include "shared/trees/red-black-memory-manager.h"
#include "shared/trees/red-black/tests/assert-basic.h"
#include "shared/trees/red-black/tests/assert-memory-manager.h"
#include "shared/trees/red-black/tests/assert.h"

typedef enum { INSERT, DELETE_AT_LEAST } OperationType;

typedef struct {
    Memory memory;
    OperationType type;
} TreeOperation;

typedef ARRAY(TreeOperation) TreeOperation_a;
typedef ARRAY(TreeOperation_a) TestCases;

static TreeOperation_a noOperations = {.len = 0, .buf = 0};
static TestCases noOperationsTestCase = {.buf = &noOperations, .len = 1};

static TreeOperation insert1[] = {
    {{.start = 1000, .bytes = 100}, INSERT},
    {{.start = 1100, .bytes = 100}, INSERT},
    {{.start = 1500, .bytes = 200}, INSERT},
    {{.start = 2000, .bytes = 300}, INSERT},
    {{.start = 2500, .bytes = 100}, INSERT},
    {{.start = 3000, .bytes = 100}, INSERT},
    {{.start = 3300, .bytes = 100}, INSERT},
    {{.start = 2900, .bytes = 100}, INSERT},
    {{.start = 3150, .bytes = 100}, INSERT},
    {{.start = 500, .bytes = 499}, INSERT},
    {{.start = 2400, .bytes = 50}, INSERT},
    {{.start = 1700, .bytes = 300}, INSERT},
};

static TreeOperation insert2[] = {
    {{.start = 5000, .bytes = 128}, INSERT},
    {{.start = 4872, .bytes = 128}, INSERT},
    {{.start = 5128, .bytes = 128}, INSERT},
    {{.start = 6000, .bytes = 256}, INSERT},
    {{.start = 7000, .bytes = 128}, INSERT},
    {{.start = 7500, .bytes = 128}, INSERT},
    {{.start = 5256, .bytes = 744}, INSERT},
    {{.start = 8000, .bytes = 128}, INSERT},
    {{.start = 8300, .bytes = 128}, INSERT},
    {{.start = 8600, .bytes = 128}, INSERT},
    {{.start = 8900, .bytes = 999999999}, INSERT}};

static TreeOperation insert3[] = {{{.start = 10000, .bytes = 512}, INSERT},
                                  {{.start = 10512, .bytes = 512}, INSERT},
                                  {{.start = 11024, .bytes = 256}, INSERT},
                                  {{.start = 11500, .bytes = 500}, INSERT},
                                  {{.start = 12000, .bytes = 500}, INSERT},
                                  {{.start = 13500, .bytes = 1000}, INSERT},
                                  {{.start = 14500, .bytes = 500}, INSERT},
                                  {{.start = 15000, .bytes = 256}, INSERT},
                                  {{.start = 18063, .bytes = 12345567}, INSERT},
                                  {{.start = 11280, .bytes = 220}, INSERT}};
static TreeOperation_a inserts[] = {{.buf = insert1, .len = COUNTOF(insert1)},
                                    {.buf = insert2, .len = COUNTOF(insert2)},
                                    {.buf = insert3, .len = COUNTOF(insert3)}};
static constexpr auto INSERTS_TEST_CASES_LEN = COUNTOF(inserts);
static TestCases insertsOnlyTestCases = {.buf = inserts,
                                         .len = INSERTS_TEST_CASES_LEN};

static TreeOperation insertDeleteAtLeast1[] = {
    {{.start = 1000, .bytes = 100}, INSERT},
    {{.start = 1100, .bytes = 100}, INSERT},
    {{.start = 1500, .bytes = 200}, INSERT},
    {{.start = 2000, .bytes = 300}, INSERT},
    {{.start = 2500, .bytes = 100}, INSERT},
    {{.start = 3000, .bytes = 100}, INSERT},
    {{.start = 3300, .bytes = 100}, INSERT},
    {{.start = 2900, .bytes = 100}, INSERT},
    {{.start = 3150, .bytes = 100}, INSERT},
    {{.start = 500, .bytes = 499}, INSERT},
    {{.start = 2400, .bytes = 50}, INSERT},
    {{.start = 0, .bytes = 500}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 100}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 100}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 300}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 76}, DELETE_AT_LEAST},
    {{.start = 1700, .bytes = 300}, INSERT}};
static TreeOperation insertDeleteAtLeast2[] = {
    {{.start = 5000, .bytes = 128}, INSERT},
    {{.start = 4872, .bytes = 128}, INSERT},
    {{.start = 5128, .bytes = 128}, INSERT},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 6000, .bytes = 256}, INSERT},
    {{.start = 7000, .bytes = 128}, INSERT},
    {{.start = 7500, .bytes = 128}, INSERT},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 132}, DELETE_AT_LEAST},
    {{.start = 5256, .bytes = 744}, INSERT},
    {{.start = 8000, .bytes = 128}, INSERT},
    {{.start = 8300, .bytes = 128}, INSERT},
    {{.start = 8600, .bytes = 128}, INSERT},
    {{.start = 8900, .bytes = 999999999}, INSERT},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 356786}, DELETE_AT_LEAST},
};
static TreeOperation insertDeleteAtLeast3[] = {
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
    {{.start = 10000, .bytes = 512}, INSERT},
    {{.start = 10512, .bytes = 512}, INSERT},
    {{.start = 11024, .bytes = 256}, INSERT},
    {{.start = 11500, .bytes = 500}, INSERT},
    {{.start = 12000, .bytes = 500}, INSERT},
    {{.start = 0, .bytes = 500}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 500}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 500}, DELETE_AT_LEAST},
    {{.start = 0, .bytes = 500}, DELETE_AT_LEAST},
    {{.start = 13500, .bytes = 1000}, INSERT},
    {{.start = 14500, .bytes = 500}, INSERT},
    {{.start = 15000, .bytes = 256}, INSERT},
    {{.start = 18063, .bytes = 12345567}, INSERT},
    {{.start = 11280, .bytes = 220}, INSERT},
    {{.start = 0, .bytes = 10}, DELETE_AT_LEAST},
};
static TreeOperation_a insertDeleteAtLeasts[] = {
    {.buf = insertDeleteAtLeast1, .len = COUNTOF(insertDeleteAtLeast1)},
    {.buf = insertDeleteAtLeast2, .len = COUNTOF(insertDeleteAtLeast2)},
    {.buf = insertDeleteAtLeast3, .len = COUNTOF(insertDeleteAtLeast3)}};
static constexpr auto INSERT_DELETE_AT_LEASTS_TEST_CASES_LEN =
    COUNTOF(insertDeleteAtLeasts);
static TestCases insertDeleteAtLeastsOnlyTestCases = {
    .buf = insertDeleteAtLeasts, .len = INSERT_DELETE_AT_LEASTS_TEST_CASES_LEN};

static void addValueToExpected(Memory_max_a *expectedValues, Memory toAdd,
                               RedBlackNodeMM *tree) {
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
                INFO(MAX_NODES_IN_TREE, NEWLINE);
                appendRedBlackTreeWithBadNode((RedBlackNode *)tree, nullptr,
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
    RedBlackNodeMM *tree = nullptr;
    Memory_max_a expectedValues =
        (Memory_max_a){.buf = NEW(&scratch, Memory, MAX_NODES_IN_TREE),
                       .len = 0,
                       .cap = MAX_NODES_IN_TREE};
    for (U64 i = 0; i < operations.len; i++) {
        switch (operations.buf[i].type) {
        case INSERT: {
            addValueToExpected(&expectedValues, operations.buf[i].memory, tree);

            RedBlackNodeMM *createdNode = NEW(&scratch, RedBlackNodeMM);
            createdNode->memory = operations.buf[i].memory;
            insertRedBlackNodeMM(&tree, createdNode);
            break;
        }
        case DELETE_AT_LEAST: {
            RedBlackNodeMM *deleted = deleteAtLeastRedBlackNodeMM(
                &tree, operations.buf[i].memory.bytes);
            if (!deleted) {
                for (U64 j = 0; j < expectedValues.len; j++) {
                    if (expectedValues.buf[j].bytes >=
                        operations.buf[i].memory.bytes) {
                        TEST_FAILURE {
                            INFO(STRING("Did not find a node todelete value "));
                            INFO(operations.buf[i].memory.bytes);
                            INFO(STRING(
                                ", should have deleted node with value: "));
                            INFO(expectedValues.buf[i].bytes, NEWLINE);
                        }
                    }
                }
                break;
            } else if (deleted->memory.bytes < operations.buf[i].memory.bytes) {
                TEST_FAILURE {
                    INFO(STRING("Deleted value not equal the value that should "
                                "have been deleted !\nExpected to be "
                                "deleted value: "));
                    INFO(operations.buf[i].memory.bytes, NEWLINE);
                    INFO(STRING("Actual deleted value: "));
                    INFO(deleted->memory.bytes, NEWLINE);
                }
            } else {
                U64 indexToRemove = 0;
                for (U64 j = 0; j < expectedValues.len; j++) {
                    if (expectedValues.buf[j].start == deleted->memory.start) {
                        indexToRemove = j;
                    }
                }

                expectedValues.buf[indexToRemove] =
                    expectedValues.buf[expectedValues.len - 1];
                expectedValues.len--;
                break;
            }
        }
        }

        assertMMRedBlackTreeValid(tree, expectedValues, scratch);
    }

    testSuccess();
}

static void testSubTopic(string subTopic, TestCases testCases, Arena scratch) {
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
