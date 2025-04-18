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

static TreeOperation mixed7[] = {
    {11111, INSERT},          {22222, INSERT},         {33333, INSERT},
    {44444, INSERT},          {55555, INSERT},         {66666, INSERT},
    {77777, INSERT},          {88888, INSERT},         {99999, INSERT},
    {12345, INSERT},          {98765, INSERT},         {55555, DELETE},
    {88880, DELETE_AT_LEAST}, {11111, DELETE_AT_LEAST}};
static TreeOperation mixed8[] = {
    {33333, INSERT}, {44444, INSERT},          {55555, INSERT},
    {55554, INSERT}, {55554, DELETE_AT_LEAST}, {55554, DELETE_AT_LEAST},
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

    {.name = STRING("Mixed operations 7"),
     .operations = {.buf = mixed7, .len = COUNTOF(mixed7)}},
    {.name = STRING("Mixed operations 8"),
     .operations = {.buf = mixed8, .len = COUNTOF(mixed8)}},
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
                    printRedBlackTreeWithBadNode(tree, nullptr);
                }
                return;
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
                return;
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
                        return;
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
                return;
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

        if (!assertRedBlackTreeValid(tree, expectedValues, scratch)) {
            return;
        }
    }

    testSuccess();
}

void testRedBlackTrees(Arena scratch) {
    TEST_TOPIC(STRING("Operations")) {
        for (U64 i = 0; i < TEST_CASES_LEN; i++) {
            TEST(testCases[i].name) {
                testTree(testCases[i].operations, scratch);
            }
        }
    }
}
