#include "shared/trees/red-black/tests/insert.h"
#include "posix/test-framework/test.h"
#include "shared/macros.h"
#include "shared/memory/allocator/macros.h"
#include "shared/trees/red-black.h"
#include "shared/trees/red-black/tests/correct.h"

typedef struct {
    string name;
    U64_a array;
} TestInsert;

static U64 insert_1[] = {37, 14, 25, 1, 18, 50, 42, 26, 9, 12, 38};
static U64 insert_2[] = {100, 50, 25, 75, 10, 40, 60, 80, 90, 30, 20};
static U64 insert_3[] = {1999, 2021, 100, 300, 150, 1200, 50, 987, 6000, 5678};
static U64 insert_4[] = {873, 582, 312, 54, 712, 946, 402, 833, 166};
static U64 insert_5[] = {550, 367, 472, 812, 1500, 689, 1450, 23, 875};
static U64 insert_6[] = {215, 778, 144, 310, 188, 50,
                         25,  563, 137, 980, 249, 500};
static U64 insert_7[] = {45, 16, 78, 34, 52, 67, 89, 25, 12, 40, 3, 56};
static U64 insert_8[] = {888, 543, 555, 234, 984, 432, 1, 999, 777, 900};
static U64 insert_9[] = {122, 45, 67, 89, 12, 35, 56, 13, 78, 99, 111};
static U64 insert_10[] = {11111, 22222, 33333, 44444, 55555, 66666,
                          77777, 88888, 99999, 12345, 98765};
static U64 insert_11[] = {};

static TestInsert testCases[] = {
    {.name = STRING("Test 1"),
     .array = {.buf = insert_1, .len = COUNTOF(insert_1)}},
    {.name = STRING("Test 2"),
     .array = {.buf = insert_2, .len = COUNTOF(insert_2)}},
    {.name = STRING("Test 3"),
     .array = {.buf = insert_3, .len = COUNTOF(insert_3)}},
    {.name = STRING("Test 4"),
     .array = {.buf = insert_4, .len = COUNTOF(insert_4)}},
    {.name = STRING("Test 5"),
     .array = {.buf = insert_5, .len = COUNTOF(insert_5)}},
    {.name = STRING("Test 6"),
     .array = {.buf = insert_6, .len = COUNTOF(insert_6)}},
    {.name = STRING("Test 7"),
     .array = {.buf = insert_7, .len = COUNTOF(insert_7)}},
    {.name = STRING("Test 8"),
     .array = {.buf = insert_8, .len = COUNTOF(insert_8)}},
    {.name = STRING("Test 9"),
     .array = {.buf = insert_9, .len = COUNTOF(insert_9)}},
    {.name = STRING("Test 10"),
     .array = {.buf = insert_10, .len = COUNTOF(insert_10)}},
    {.name = STRING("Null tree"),
     .array = {.buf = insert_11, .len = COUNTOF(insert_11)}},
};
static constexpr auto TEST_CASES_LEN = COUNTOF(testCases);

void testRedBlackTreeInserts(Arena scratch) {
    TEST_TOPIC(STRING("Inserts")) {
        for (U64 i = 0; i < TEST_CASES_LEN; i++) {
            TEST(testCases[i].name) {
                Arena localArena = scratch;
                RedBlackNode *tree = nullptr;
                for (U64 j = 0; j < testCases[i].array.len; j++) {
                    RedBlackNode *createdNode =
                        NEW(&localArena, RedBlackNode, 1, ZERO_MEMORY);
                    createdNode->value = testCases[i].array.buf[j];
                    insertRedBlackNode(&tree, createdNode);
                }
                assertRedBlackTreeValid(tree, localArena);
            }
        }
    }
}
