#include "shared/trees/red-black/tests/red-black/basic.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/memory/allocator/macros.h"
#include "shared/text/string.h"
#include "shared/trees/red-black/basic.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/trees/red-black/tests/assert-basic.h"
#include "shared/trees/red-black/tests/assert.h"
#include "shared/trees/red-black/tests/red-black/common.h"
#include "shared/trees/red-black/virtual-mapping-manager.h"

typedef enum { INSERT, DELETE, DELETE_AT_LEAST } OperationType;

typedef struct {
    U64 value;
    OperationType type;
} TreeOperation;

typedef ARRAY(TreeOperation) TreeOperation_a;
typedef ARRAY(TreeOperation_a) TestCases;

static TreeOperation_a noOperations = {.buf = 0, .len = 0};
static TestCases noOperationsTestCase = {.buf = &noOperations, .len = 1};

static TreeOperation insert1[] = {{37, INSERT}, {14, INSERT}, {25, INSERT},
                                  {1, INSERT},  {18, INSERT}, {50, INSERT},
                                  {42, INSERT}, {26, INSERT}, {9, INSERT},
                                  {12, INSERT}, {38, INSERT}};
static TreeOperation insert2[] = {{100, INSERT}, {50, INSERT}, {25, INSERT},
                                  {75, INSERT},  {10, INSERT}, {40, INSERT},
                                  {60, INSERT},  {80, INSERT}, {90, INSERT},
                                  {30, INSERT},  {20, INSERT}};
static TreeOperation insert3[] = {
    {1999, INSERT}, {2021, INSERT}, {100, INSERT},  {300, INSERT},
    {150, INSERT},  {1200, INSERT}, {50, INSERT},   {987, INSERT},
    {6000, INSERT}, {5678, INSERT}, {7000, INSERT}, {8000, INSERT},
    {9000, INSERT}, {9001, INSERT}, {9002, INSERT}, {9003, INSERT},
    {9004, INSERT}, {9005, INSERT}, {9006, INSERT}, {9007, INSERT},
    {9008, INSERT}, {9009, INSERT}, {9010, INSERT}, {9011, INSERT},
    {9012, INSERT}, {9013, INSERT}, {9014, INSERT}, {9015, INSERT},
    {9016, INSERT}, {9017, INSERT}, {9018, INSERT}, {9019, INSERT},
};
static TreeOperation_a inserts[] = {
    {.buf = insert1, .len = COUNTOF(insert1)},
    {.buf = insert2, .len = COUNTOF(insert2)},
    {.buf = insert3, .len = COUNTOF(insert3)},
};
static constexpr auto INSERTS_TEST_CASES_LEN = COUNTOF(inserts);
static TestCases insertsOnlyTestCases = {.buf = inserts,
                                         .len = INSERTS_TEST_CASES_LEN};

static TreeOperation insertDelete1[] = {
    {873, INSERT}, {582, INSERT}, {312, INSERT}, {54, INSERT},
    {712, INSERT}, {946, INSERT}, {402, INSERT}, {833, INSERT},
    {166, INSERT}, {712, DELETE}, {582, DELETE}, {946, DELETE}};
static TreeOperation insertDelete2[] = {
    {550, INSERT},  {367, INSERT}, {472, INSERT},  {812, INSERT},
    {1500, INSERT}, {689, INSERT}, {1450, INSERT}, {23, INSERT},
    {875, INSERT},  {812, DELETE}, {367, DELETE},  {23, DELETE}};
static TreeOperation insertDelete3[] = {
    {215, INSERT}, {778, INSERT}, {144, INSERT}, {310, INSERT}, {188, INSERT},
    {50, INSERT},  {25, INSERT},  {563, INSERT}, {137, INSERT}, {980, INSERT},
    {249, INSERT}, {500, INSERT}, {215, DELETE}, {144, DELETE}, {980, DELETE}};
static TreeOperation insertDelete4[] = {{100, INSERT}, {100, DELETE}};
static TreeOperation insertDelete5[] = {
    {100, INSERT}, {75, INSERT}, {100, DELETE}};
static TreeOperation insertDelete6[] = {
    {100, INSERT}, {125, INSERT}, {100, DELETE}};
static TreeOperation insertDelete7[] = {
    {100, INSERT}, {125, INSERT}, {75, INSERT}, {100, DELETE}};
static TreeOperation_a insertsAndDeletes[] = {
    {.buf = insertDelete1, .len = COUNTOF(insertDelete1)},
    {.buf = insertDelete2, .len = COUNTOF(insertDelete2)},
    {.buf = insertDelete3, .len = COUNTOF(insertDelete3)},
    {.buf = insertDelete4, .len = COUNTOF(insertDelete4)},
    {.buf = insertDelete5, .len = COUNTOF(insertDelete5)},
    {.buf = insertDelete6, .len = COUNTOF(insertDelete6)},
    {.buf = insertDelete7, .len = COUNTOF(insertDelete7)},
};
static constexpr auto INSERTS_AND_DELETES_TEST_CASES_LEN =
    COUNTOF(insertsAndDeletes);
static TestCases insertsDeletionsTestCases = {
    .buf = insertsAndDeletes, .len = INSERTS_AND_DELETES_TEST_CASES_LEN};

static TreeOperation insertDeleteAtLeast1[] = {
    {45, INSERT},          {16, INSERT},          {78, INSERT},
    {34, INSERT},          {52, INSERT},          {67, INSERT},
    {89, INSERT},          {25, INSERT},          {12, INSERT},
    {40, INSERT},          {3, INSERT},           {56, INSERT},
    {20, DELETE_AT_LEAST}, {88, DELETE_AT_LEAST}, {2, DELETE_AT_LEAST},
    {80, DELETE_AT_LEAST}, {16, DELETE_AT_LEAST}};
static TreeOperation insertDeleteAtLeast2[] = {
    {888, INSERT},        {543, INSERT},          {555, INSERT},
    {234, INSERT},        {984, INSERT},          {432, INSERT},
    {1, INSERT},          {999, INSERT},          {777, INSERT},
    {900, INSERT},        {778, DELETE_AT_LEAST}, {1, DELETE_AT_LEAST},
    {1, DELETE_AT_LEAST}, {999, DELETE_AT_LEAST}};
static TreeOperation insertDeleteAtLeast3[] = {
    {122, INSERT},         {45, INSERT},          {67, INSERT},
    {89, INSERT},          {12, INSERT},          {35, INSERT},
    {56, INSERT},          {13, INSERT},          {78, INSERT},
    {99, INSERT},          {111, INSERT},         {35, DELETE_AT_LEAST},
    {45, DELETE_AT_LEAST}, {70, DELETE_AT_LEAST}, {111, DELETE_AT_LEAST}};
static TreeOperation_a insertsAndDeleteAtLeasts[] = {
    {.buf = insertDeleteAtLeast1, .len = COUNTOF(insertDeleteAtLeast1)},
    {.buf = insertDeleteAtLeast2, .len = COUNTOF(insertDeleteAtLeast2)},
    {.buf = insertDeleteAtLeast3, .len = COUNTOF(insertDeleteAtLeast3)},
};
static constexpr auto INSERTS_AND_DELETE_AT_LEASTS_TEST_CASES_LEN =
    COUNTOF(insertsAndDeleteAtLeasts);
static TestCases insertsDeletionAtLeastsTestCases = {
    .buf = insertsAndDeleteAtLeasts,
    .len = INSERTS_AND_DELETE_AT_LEASTS_TEST_CASES_LEN};

static TreeOperation mixed1[] = {
    {11111, INSERT},          {22222, INSERT},         {33333, INSERT},
    {44444, INSERT},          {55555, INSERT},         {66666, INSERT},
    {77777, INSERT},          {88888, INSERT},         {99999, INSERT},
    {12345, INSERT},          {98765, INSERT},         {55555, DELETE},
    {88880, DELETE_AT_LEAST}, {11111, DELETE_AT_LEAST}};
static TreeOperation mixed2[] = {
    {33333, INSERT}, {44444, INSERT},          {55555, INSERT},
    {55554, INSERT}, {55554, DELETE_AT_LEAST}, {55554, DELETE_AT_LEAST},
};
static TreeOperation mixed3[] = {
    {622592, INSERT},
    {7340032, INSERT},
    {12288, INSERT},
    {16384, INSERT},
    {12582912, INSERT},
    {446914560, INSERT},
    {131072, INSERT},
    {696320, INSERT},
    {30175232, INSERT},
    {20480, INSERT},
    {1048576, INSERT},
    {65536, INSERT},
    {2723840, INSERT},
    {6340608, INSERT},
    {737280, INSERT},
    {401408, INSERT},
    {655360, INSERT},
    {184320, INSERT},
    {36864, INSERT},
    {20480, INSERT},
    {131072, INSERT},
    {8192, INSERT},
    {40960, INSERT},
    {4096, INSERT},
    {32768, INSERT},
    {4096, INSERT},
    {65536, INSERT},
    {12288, INSERT},
    {12288, INSERT},
    {28672, INSERT},
    {262144, INSERT},
    {57344, INSERT},
    {61440, INSERT},
    {8192, INSERT},
    {86016, INSERT},
    {8192, INSERT},
    {81920, INSERT},
    {28672, INSERT},
    {143360, INSERT},
    {12288, INSERT},
    {45056, INSERT},
    {4096, INSERT},
    {12288, INSERT},
    {12288, INSERT},
    {442368, INSERT},
    {2097152, INSERT},
    {28672, INSERT},
    {8192, INSERT},
    {53248, INSERT},
    {12288, INSERT},
    {8192, INSERT},
    {8192, INSERT},
    {16384, INSERT},
    {4096, INSERT},
    {24576, INSERT},
    {16384, INSERT},
    {57344, INSERT},
    {8192, INSERT},
    {135168, INSERT},
    {12288, INSERT},
    {16384, INSERT},
    {8192, INSERT},
    {81920, INSERT},
    {16384, INSERT},
    {4096, INSERT},
    {4096, INSERT},
    {28672, INSERT},
    {32768, INSERT},
    {16384, INSERT},
    {28672, INSERT},
    {16384, INSERT},
    {12288, INSERT},
    {40960, INSERT},
    {4096, INSERT},
    {143360, INSERT},
    {4096, INSERT},
    {32768, INSERT},
    {32768, INSERT},
    {4194304, INSERT},
    {458752, INSERT},
    {12288, INSERT},
    {24576, INSERT},
    {12288, INSERT},
    {16384, INSERT},
    {16384, INSERT},
    {90112, INSERT},
    {4096, INSERT},
    {4096, INSERT},
    {4096, INSERT},
    {8192, INSERT},
    {8192, INSERT},
    {36864, INSERT},
    {4096, INSERT},
    {4096, INSERT},
    {8192, INSERT},
    {4096, INSERT},
    {4096, INSERT},
    {12288, INSERT},
    {45056, INSERT},
    {53248, INSERT},
    {4096, INSERT},
    {4096, INSERT},
    {8192, INSERT},
    {4096, INSERT},
    {8192, INSERT},
    {40960, INSERT},
    {4096, INSERT},
    {8192, INSERT},
    {4096, INSERT},
    {36864, INSERT},
    {16384, INSERT},
    {12288, INSERT},
    {4579328, INSERT},
    {2101248, INSERT},
    {864256, INSERT},
    {131072, INSERT},
    {159744, INSERT},
    {36864, INSERT},
    {94209, INSERT},
    {1656, DELETE_AT_LEAST},
    {2440, INSERT},
    {1048576, DELETE_AT_LEAST},
    {16777216, DELETE_AT_LEAST},
    {13340672, INSERT},
    {10516992, INSERT},
    {8, INSERT},
};
static TreeOperation_a mixed[] = {
    {.buf = mixed1, .len = COUNTOF(mixed1)},
    {.buf = mixed2, .len = COUNTOF(mixed2)},
    {.buf = mixed3, .len = COUNTOF(mixed3)},
};
static constexpr auto MIXED_TEST_CASES_LEN = COUNTOF(mixed);
static TestCases mixedTestCases = {.buf = mixed, .len = MIXED_TEST_CASES_LEN};

static void testTree(TreeOperation_a operations, Arena scratch) {
    VMMTreeWithFreeList treeWithFreeList = {
        .buf = NEW(&scratch, VMMNode, .count = MAX_NODES_IN_TREE),
        .len = 0,
        .cap = MAX_NODES_IN_TREE,
        .tree = 0,
        .elementSizeBytes = sizeof(*treeWithFreeList.buf),
        .freeList =
            (U32_max_a){.buf = NEW(&scratch, U32, .count = MAX_NODES_IN_TREE),
                        .len = 0,
                        .cap = MAX_NODES_IN_TREE}};
    treeWithFreeList.buf[0] = (VMMNode){0};
    treeWithFreeList.len = 1;

    U64_max_a expectedValues =
        (U64_max_a){.buf = NEW(&scratch, U64, .count = MAX_NODES_IN_TREE),
                    .len = 0,
                    .cap = MAX_NODES_IN_TREE};

    for (U64 i = 0; i < operations.len; i++) {
        switch (operations.buf[i].type) {
        case INSERT: {
            VMMNode *createdNode =
                getFromNodes((TreeWithFreeList *)&treeWithFreeList);
            createdNode->data.memory.start = operations.buf[i].value;
            createdNode->data.memory.bytes = 1;
            createdNode->data.mappingSize = 4096;

            (void)insertVMMNode(&treeWithFreeList, createdNode);

            if (expectedValues.len >= MAX_NODES_IN_TREE) {
                TEST_FAILURE {
                    INFO(STRING("Tree contains too many nodes to fit in array. "
                                "Increase max size or decrease expected nodes "
                                "in Red-Black tree. Current maximum size: "));
                    INFO(MAX_NODES_IN_TREE, .flags = NEWLINE);
                    appendRedBlackTreeWithBadNode(
                        (TreeWithFreeList *)&treeWithFreeList, 0,
                        RED_BLACK_VIRTUAL_MEMORY_MAPPER);
                }
            }
            expectedValues.buf[expectedValues.len] = operations.buf[i].value;
            expectedValues.len++;
            break;
        }
        case DELETE: {
            U32 deleted =
                deleteVMMNode(&treeWithFreeList, operations.buf[i].value);
            VMMNode *deletedNode = getVMMNode(&treeWithFreeList, deleted);
            if (deletedNode->data.memory.start != operations.buf[i].value) {
                TEST_FAILURE {
                    INFO(STRING("Deleted value does not equal the value that "
                                "should have been deleted!\nExpected to be "
                                "deleted value: "));
                    INFO(operations.buf[i].value, .flags = NEWLINE);
                    INFO(STRING("Actual deleted value: "));
                    INFO(deletedNode->data.memory.start, .flags = NEWLINE);
                }
            }

            U64 indexToRemove = 0;
            for (U64 j = 0; j < expectedValues.len; j++) {
                if (expectedValues.buf[j] == operations.buf[i].value) {
                    indexToRemove = j;
                    break;
                }
            }

            expectedValues.buf[indexToRemove] =
                expectedValues.buf[expectedValues.len - 1];
            expectedValues.len--;
            break;
        }
        case DELETE_AT_LEAST: {
            U32 deleted = deleteAtLeastVMMNode(&treeWithFreeList,
                                               operations.buf[i].value);
            if (!deleted) {
                for (U64 j = 0; j < expectedValues.len; j++) {
                    if (expectedValues.buf[j] >= operations.buf[i].value) {
                        TEST_FAILURE {
                            INFO(
                                STRING("Did not find a node to delete value "));
                            INFO(operations.buf[i].value);
                            INFO(STRING(
                                ", should have deleted node with value: "));
                            INFO(expectedValues.buf[i], .flags = NEWLINE);
                        }
                    }
                }
                break;
            } else {
                VMMNode *deletedNode = getVMMNode(&treeWithFreeList, deleted);
                if (deletedNode->data.memory.start < operations.buf[i].value) {
                    TEST_FAILURE {
                        INFO(STRING("Deleted value not equal the value that "
                                    "should have been deleted!\nExpected to be "
                                    "deleted value: "));
                        INFO(operations.buf[i].value, .flags = NEWLINE);
                        INFO(STRING("Actual deleted value: "));
                        INFO(deletedNode->data.memory.start, .flags = NEWLINE);
                    }
                } else {
                    U64 indexToRemove = 0;
                    U64 valueToRemove = U64_MAX;
                    for (U64 j = 0; j < expectedValues.len; j++) {
                        if (expectedValues.buf[j] >= operations.buf[i].value) {
                            if (expectedValues.buf[j] < valueToRemove) {
                                indexToRemove = j;
                                valueToRemove = expectedValues.buf[j];
                            }
                        }
                    }

                    expectedValues.buf[indexToRemove] =
                        expectedValues.buf[expectedValues.len - 1];
                    expectedValues.len--;
                    break;
                }
            }
        }
        }

        assertBasicRedBlackTreeValid(&treeWithFreeList, expectedValues,
                                     scratch);
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

void testBasicRedBlackTrees(Arena scratch) {
    TEST_TOPIC(STRING("Basic red-black trees")) {
        testSubTopic(STRING("No Operations"), noOperationsTestCase, scratch);
        testSubTopic(STRING("Inserts Only"), insertsOnlyTestCases, scratch);
        testSubTopic(STRING("Inserts + Deletions"), insertsDeletionsTestCases,
                     scratch);
        testSubTopic(STRING("Inserts + At Least Deletions"),
                     insertsDeletionAtLeastsTestCases, scratch);
        testSubTopic(STRING("Mixed"), mixedTestCases, scratch);
    }
}
