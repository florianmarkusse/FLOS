#include "shared/trees/red-black/tests/test.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/memory/allocator/macros.h"
#include "shared/trees/red-black.h"
#include "shared/trees/red-black/tests/correct.h"

typedef enum { INSERT, DELETE, DELETE_AT_LEAST } OperationType;

typedef struct {
    U64 value;
    OperationType type;
} TreeOperation;

typedef ARRAY(TreeOperation) TreeOperation_a;

typedef struct {
    string name;
    TreeOperation_a operations;
} TestCase;

static TreeOperation noOperations[] = {
    // Empty test case
};

static TreeOperation insert1[] = {{37, INSERT}, {14, INSERT}, {25, INSERT},
                                  {1, INSERT},  {18, INSERT}, {50, INSERT},
                                  {42, INSERT}, {26, INSERT}, {9, INSERT},
                                  {12, INSERT}, {38, INSERT}};
static TreeOperation insert2[] = {{100, INSERT}, {50, INSERT}, {25, INSERT},
                                  {75, INSERT},  {10, INSERT}, {40, INSERT},
                                  {60, INSERT},  {80, INSERT}, {90, INSERT},
                                  {30, INSERT},  {20, INSERT}};
static TreeOperation insert3[] = {
    {1999, INSERT}, {2021, INSERT}, {100, INSERT}, {300, INSERT},
    {150, INSERT},  {1200, INSERT}, {50, INSERT},  {987, INSERT},
    {6000, INSERT}, {5678, INSERT}};

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

static TestCase testCases[] = {
    {.name = STRING("No operations"),
     .operations = {.buf = noOperations, .len = COUNTOF(noOperations)}},

    {.name = STRING("Insert only 1"),
     .operations = {.buf = insert1, .len = COUNTOF(insert1)}},
    {.name = STRING("Insert only 2"),
     .operations = {.buf = insert2, .len = COUNTOF(insert2)}},
    {.name = STRING("Insert only 3"),
     .operations = {.buf = insert3, .len = COUNTOF(insert3)}},

    {.name = STRING("Insert + Delete 1"),
     .operations = {.buf = insertDelete1, .len = COUNTOF(insertDelete1)}},
    {.name = STRING("Insert + Delete 2"),
     .operations = {.buf = insertDelete2, .len = COUNTOF(insertDelete2)}},
    {.name = STRING("Insert + Delete 3"),
     .operations = {.buf = insertDelete3, .len = COUNTOF(insertDelete3)}},

    {.name = STRING("Insert + Delete At Least 1"),
     .operations = {.buf = insertDeleteAtLeast1,
                    .len = COUNTOF(insertDeleteAtLeast1)}},
    {.name = STRING("Insert + Delete At Least 2"),
     .operations = {.buf = insertDeleteAtLeast2,
                    .len = COUNTOF(insertDeleteAtLeast2)}},
    {.name = STRING("Insert + Delete At Least 3"),
     .operations = {.buf = insertDeleteAtLeast3,
                    .len = COUNTOF(insertDeleteAtLeast3)}},

    {.name = STRING("Mixed operations 1"),
     .operations = {.buf = mixed1, .len = COUNTOF(mixed1)}},
    {.name = STRING("Mixed operations 2"),
     .operations = {.buf = mixed2, .len = COUNTOF(mixed2)}},
    {.name = STRING("Mixed operations 3"),
     .operations = {.buf = mixed3, .len = COUNTOF(mixed3)}},
};
static constexpr auto TEST_CASES_LEN = COUNTOF(testCases);

static constexpr auto MAX_NODES_IN_TREE = 1024;

static void testTree(TreeOperation_a operations, Arena scratch) {
    RedBlackNode *tree = nullptr;
    U64_max_a expectedValues =
        (U64_max_a){.buf = NEW(&scratch, U64, MAX_NODES_IN_TREE),
                    .len = 0,
                    .cap = MAX_NODES_IN_TREE};
    for (U64 i = 0; i < operations.len; i++) {
        switch (operations.buf[i].type) {
        case INSERT: {
            RedBlackNode *createdNode = NEW(&scratch, RedBlackNode);
            createdNode->memory.bytes = operations.buf[i].value;
            insertRedBlackNode(&tree, createdNode);

            if (expectedValues.len >= MAX_NODES_IN_TREE) {
                TEST_FAILURE {
                    INFO(STRING("Tree contains too many nodes to fit in array. "
                                "Increase max size or decrease expected nodes "
                                "in Red-Black tree. Current maximum size: "));
                    INFO(MAX_NODES_IN_TREE, NEWLINE);
                    appendRedBlackTreeWithBadNode(tree, nullptr);
                }
            }
            expectedValues.buf[expectedValues.len] = operations.buf[i].value;
            expectedValues.len++;
            break;
        }
        case DELETE: {
            RedBlackNode *deleted =
                deleteRedBlackNode(&tree, operations.buf[i].value);
            if (deleted->memory.bytes != operations.buf[i].value) {
                TEST_FAILURE {
                    INFO(STRING("Deleted value does not equal the value that "
                                "should have been deleted!\nExpected to be "
                                "deleted value: "));
                    INFO(operations.buf[i].value, NEWLINE);
                    INFO(STRING("Actual deleted value: "));
                    INFO(deleted->memory.bytes, NEWLINE);
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
            RedBlackNode *deleted =
                deleteAtLeastRedBlackNode(&tree, operations.buf[i].value);
            if (!deleted) {
                for (U64 j = 0; j < expectedValues.len; j++) {
                    if (expectedValues.buf[j] >= operations.buf[i].value) {
                        TEST_FAILURE {
                            INFO(
                                STRING("Did not find a node to delete value "));
                            INFO(operations.buf[i].value);
                            INFO(STRING(
                                ", should have deleted node with value: "));
                            INFO(operations.buf[i].value, NEWLINE);
                        }
                    }
                }
                break;
            } else if (deleted->memory.bytes < operations.buf[i].value) {
                TEST_FAILURE {
                    INFO(STRING("Deleted value not equal the value that "
                                "should have been deleted!\nExpected to be "
                                "deleted value: "));
                    INFO(operations.buf[i].value, NEWLINE);
                    INFO(STRING("Actual deleted value: "));
                    INFO(deleted->memory.bytes, NEWLINE);
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

        assertRedBlackTreeValid(tree, expectedValues, scratch);
    }

    testSuccess();
}

void testRedBlackTrees(Arena scratch) {
    TEST_TOPIC(STRING("Operations")) {
        JumpBuffer failureHandler;
        for (U64 i = 0; i < TEST_CASES_LEN; i++) {
            if (setjmp(failureHandler)) {
                continue;
            }
            TEST(testCases[i].name, failureHandler) {
                testTree(testCases[i].operations, scratch);
            }
        }
    }
}
