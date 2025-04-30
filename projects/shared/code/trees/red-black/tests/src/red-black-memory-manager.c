#include "shared/trees/red-black/tests/red-black-memory-manager.h"

#include "abstraction/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/memory/allocator/macros.h"
#include "shared/text/string.h"
#include "shared/trees/red-black-basic.h"
#include "shared/trees/red-black/tests/assert-basic.h"
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

static TreeOperation insert1[] = {{{.start = 1000, .bytes = 100}, INSERT},
                                  {{.start = 1100, .bytes = 100}, INSERT},
                                  {{.start = 1500, .bytes = 200}, INSERT},
                                  {{.start = 2000, .bytes = 100}, INSERT},
                                  {{.start = 2500, .bytes = 100}, INSERT},
                                  {{.start = 1700, .bytes = 300}, INSERT},
                                  {{.start = 3000, .bytes = 100}, INSERT},
                                  {{.start = 3300, .bytes = 100}, INSERT},
                                  {{.start = 2900, .bytes = 100}, INSERT},
                                  {{.start = 3150, .bytes = 100}, INSERT},
                                  {{.start = 500, .bytes = 499}, INSERT}};

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

static void testSubTopic(string subTopic, TestCases testCases, Arena scratch) {
    TEST_TOPIC(subTopic) {
        JumpBuffer failureHandler;
        for (U64 i = 0; i < testCases.len; i++) {
            if (setjmp(failureHandler)) {
                continue;
            }
            TEST(U64ToStringDefault(i), failureHandler) {
                // testTree(testCases.buf[i], scratch);
                testFailure();
            }
        }
    }
}

void testMemoryManagerRedBlackTrees(Arena scratch) {
    TEST_TOPIC(STRING("Memory Manager red-black trees")) {
        testSubTopic(STRING("No Operations"), noOperationsTestCase, scratch);
        testSubTopic(STRING("Insert Only"), insertsOnlyTestCases, scratch);
    }
}
