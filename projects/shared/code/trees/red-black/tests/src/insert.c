#include "shared/trees/red-black/tests/insert.h"
#include "posix/test-framework/test.h"
#include "shared/macros.h"
#include "shared/trees/red-black.h"
#include "shared/trees/red-black/tests/correct.h"

typedef struct {
    string name;
    U64_a array;
} TestInsert;

static U64 insert_1[] = {1, 2, 3};
static U64 insert_2[] = {3, 2, 1};
static U64 insert_3[] = {466, 48966, 4348, 463241, 54};
static U64 insert_4[] = {8756453, 4624, 465435};
static U64 insert_5[] = {5, 9, 7};

static TestInsert testCases[] = {
    {.name = STRING("Null tree"),
     .array = {.buf = insert_1, .len = COUNTOF(insert_1)}},
    {.name = STRING("Test 2"),
     .array = {.buf = insert_2, .len = COUNTOF(insert_2)}},
    {.name = STRING("Test 3"),
     .array = {.buf = insert_3, .len = COUNTOF(insert_3)}},
    {.name = STRING("Test 4"),
     .array = {.buf = insert_4, .len = COUNTOF(insert_4)}},
    {.name = STRING("Test 5"),
     .array = {.buf = insert_5, .len = COUNTOF(insert_5)}},
};
static constexpr auto TEST_CASES_LEN = COUNTOF(testCases);

void testRedBlackTreeInserts(Arena scratch) {
    TEST_TOPIC(STRING("Inserts")) {
        for (U64 i = 0; i < TEST_CASES_LEN; i++) {
            TEST(testCases[i].name) {
                Arena localArena = scratch;
                RedBlackNode *tree = nullptr;
                for (U64 j = 0; j < testCases[i].array.len; j++) {
                    insertRedBlackNode(&tree, testCases[i].array.buf[j],
                                       &localArena);
                }
                assertRedBlackTreeValid(tree, localArena);
            }
        }
    }
}
