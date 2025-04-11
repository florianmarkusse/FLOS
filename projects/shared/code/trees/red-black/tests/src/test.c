#include "shared/trees/red-black/tests/test.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/memory/allocator/macros.h"
#include "shared/trees/red-black.h"
#include "shared/trees/red-black/tests/correct.h"

typedef enum { INSERT, DELETE } OperationType;

typedef struct {
    U64 value;
    OperationType type;
} TreeOperation;

typedef ARRAY(TreeOperation) TreeOperation_a;

typedef struct {
    string name;
    TreeOperation_a operations;
} TestCase;

// Insert-only tests (unchanged in behavior)
static TreeOperation operations1[] = {{37, INSERT}, {14, INSERT}, {25, INSERT},
                                      {1, INSERT},  {18, INSERT}, {50, INSERT},
                                      {42, INSERT}, {26, INSERT}, {9, INSERT},
                                      {12, INSERT}, {38, INSERT}};

static TreeOperation operations2[] = {{100, INSERT}, {50, INSERT}, {25, INSERT},
                                      {75, INSERT},  {10, INSERT}, {40, INSERT},
                                      {60, INSERT},  {80, INSERT}, {90, INSERT},
                                      {30, INSERT},  {20, INSERT}};

static TreeOperation operations3[] = {
    {1999, INSERT}, {2021, INSERT}, {100, INSERT}, {300, INSERT},
    {150, INSERT},  {1200, INSERT}, {50, INSERT},  {987, INSERT},
    {6000, INSERT}, {5678, INSERT}};

// Mixed insert and delete tests
static TreeOperation operations4[] = {
    {873, INSERT}, {582, INSERT}, {312, INSERT}, {54, INSERT},
    {712, INSERT}, {946, INSERT}, {402, INSERT}, {833, INSERT},
    {166, INSERT}, {712, DELETE}, {582, DELETE}, {946, DELETE}};

static TreeOperation operations5[] = {
    {550, INSERT},  {367, INSERT}, {472, INSERT},  {812, INSERT},
    {1500, INSERT}, {689, INSERT}, {1450, INSERT}, {23, INSERT},
    {875, INSERT},  {812, DELETE}, {367, DELETE},  {23, DELETE}};

static TreeOperation operations6[] = {
    {215, INSERT}, {778, INSERT}, {144, INSERT}, {310, INSERT}, {188, INSERT},
    {50, INSERT},  {25, INSERT},  {563, INSERT}, {137, INSERT}, {980, INSERT},
    {249, INSERT}, {500, INSERT}, {215, DELETE}, {144, DELETE}, {980, DELETE}};

static TreeOperation operations7[] = {
    {45, INSERT}, {16, INSERT}, {78, INSERT}, {34, INSERT},
    {52, INSERT}, {67, INSERT}, {89, INSERT}, {25, INSERT},
    {12, INSERT}, {40, INSERT}, {3, INSERT},  {56, INSERT},
    {25, DELETE}, {89, DELETE}, {3, DELETE},  {16, DELETE}};

static TreeOperation operations8[] = {
    {888, INSERT}, {543, INSERT}, {555, INSERT}, {234, INSERT}, {984, INSERT},
    {432, INSERT}, {1, INSERT},   {999, INSERT}, {777, INSERT}, {900, INSERT},
    {888, DELETE}, {1, DELETE},   {999, DELETE}};

static TreeOperation operations9[] = {
    {122, INSERT}, {45, INSERT}, {67, INSERT}, {89, INSERT}, {12, INSERT},
    {35, INSERT},  {56, INSERT}, {13, INSERT}, {78, INSERT}, {99, INSERT},
    {111, INSERT}, {35, DELETE}, {45, DELETE}, {78, DELETE}, {111, DELETE}};

static TreeOperation operations10[] = {
    {11111, INSERT}, {22222, INSERT}, {33333, INSERT}, {44444, INSERT},
    {55555, INSERT}, {66666, INSERT}, {77777, INSERT}, {88888, INSERT},
    {99999, INSERT}, {12345, INSERT}, {98765, INSERT}, {55555, DELETE},
    {88888, DELETE}, {11111, DELETE}};

static TreeOperation operations11[] = {
    // Empty test case
};

static TreeOperation operations12[] = {
    {33333, INSERT}, {44444, INSERT}, {55555, INSERT},
    {55554, INSERT}, {55555, DELETE},
};

static TestCase testCases[] = {
    {.name = STRING("Test 1"),
     .operations = {.buf = operations1, .len = COUNTOF(operations1)}},
    {.name = STRING("Test 2"),
     .operations = {.buf = operations2, .len = COUNTOF(operations2)}},
    {.name = STRING("Test 3"),
     .operations = {.buf = operations3, .len = COUNTOF(operations3)}},
    {.name = STRING("Test 4"),
     .operations = {.buf = operations4, .len = COUNTOF(operations4)}},
    {.name = STRING("Test 5"),
     .operations = {.buf = operations5, .len = COUNTOF(operations5)}},
    {.name = STRING("Test 6"),
     .operations = {.buf = operations6, .len = COUNTOF(operations6)}},
    {.name = STRING("Test 7"),
     .operations = {.buf = operations7, .len = COUNTOF(operations7)}},
    {.name = STRING("Test 8"),
     .operations = {.buf = operations8, .len = COUNTOF(operations8)}},
    {.name = STRING("Test 9"),
     .operations = {.buf = operations9, .len = COUNTOF(operations9)}},
    {.name = STRING("Test 10"),
     .operations = {.buf = operations10, .len = COUNTOF(operations10)}},
    {.name = STRING("Empty Tree"),
     .operations = {.buf = operations11, .len = COUNTOF(operations11)}},
    {.name = STRING("Test 12"),
     .operations = {.buf = operations12, .len = COUNTOF(operations12)}},
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
        if (operations.buf[i].type == INSERT) {
            RedBlackNode *createdNode = NEW(&scratch, RedBlackNode);
            createdNode->value = operations.buf[i].value;
            insertRedBlackNode(&tree, createdNode);

            if (expectedValues.len >= MAX_NODES_IN_TREE) {
                TEST_FAILURE {
                    INFO(STRING("Tree contains too many nodes to fit in array. "
                                "Increase max size or decrease expected nodes "
                                "in Red-Black tree. Current maximum size: "));
                    INFO(MAX_NODES_IN_TREE, NEWLINE);
                    printRedBlackTreeWithBadNode(tree, tree);
                }
                return;
            }
            expectedValues.buf[expectedValues.len] = operations.buf[i].value;
            expectedValues.len++;
        } else {
            RedBlackNode *deleted =
                deleteRedBlackNode(&tree, operations.buf[i].value);
            if (deleted->value != operations.buf[i].value) {
                TEST_FAILURE {
                    INFO(STRING("Deleted value does not equal the value that "
                                "should have been deleted!\nExpected to be "
                                "deleted value: "));
                    INFO(operations.buf[i].value, NEWLINE);
                    INFO(STRING("Actual deleted value: "));
                    INFO(deleted->value, NEWLINE);
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
        }

        // printRedBlackTreeWithBadNode(tree, nullptr);

        if (!assertRedBlackTreeValid(tree, expectedValues, scratch)) {
            return;
        }
    }

    testSuccess();
}

void testRedBlackTrees(Arena scratch) {
    TEST_TOPIC(STRING("Inserts")) {
        for (U64 i = 0; i < TEST_CASES_LEN; i++) {
            TEST(testCases[i].name) {
                testTree(testCases[i].operations, scratch);
            }
        }
    }
}
