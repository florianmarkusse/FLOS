#include "abstraction/log.h"
#include "abstraction/time.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/sizes.h"
#include "shared/prng/biski.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

static constexpr auto TEST_MEMORY_AMOUNT = 1 * GiB;
static constexpr auto MAX_TEST_ENTRIES = TEST_MEMORY_AMOUNT / (sizeof(U64));

static constexpr U64 PRNG_SEED = 15466503514872390148ULL;

static U64 TEST_ITERATIONS = 32;

static U64 currentTimeNanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (U64)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

typedef struct {
    U64 cycles;
    U64 millis;
} Timing;

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)

static constexpr auto GROWTH_RATE = 2;
static constexpr auto START_ENTRIES_COUNT = 8;

static U64_d_a createDynamicArray() {
    U64_d_a result;
    result.cap = START_ENTRIES_COUNT;
    result.len = 0;
    result.buf = malloc(result.cap * sizeof(U64));
    return result;
}

#define U64_D_A_APPEND(arr, val)                                               \
    do {                                                                       \
        if ((arr).len == (arr).cap) {                                          \
            (arr).cap *= (GROWTH_RATE);                                        \
            (arr).buf = realloc((arr).buf, (arr).cap * sizeof(*(arr).buf));    \
        }                                                                      \
        (arr).buf[(arr).len] = (val);                                          \
        (arr).len++;                                                           \
    } while (0)

static Timing runMappingTest(U64 arrayEntries, bool is2MiBPage,
                             bool isBaseline) {
    U64 *buffer = mmap(NULL, TEST_MEMORY_AMOUNT, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (is2MiBPage) {
        madvise(buffer, TEST_MEMORY_AMOUNT, MADV_HUGEPAGE);
    }

    if (isBaseline) {
        for (U64 i = 0; i < arrayEntries; i++) {
            buffer[i] = i;
        }
    }

    U64 startNanos = currentTimeNanos();
    U64 startCycleCount = currentCycleCounter(true, false);

    for (U64 i = 0; i < arrayEntries; i++) {
        buffer[i] = i;
    }

    U64 endCycleCount = currentCycleCounter(false, true);
    U64 endNanos = currentTimeNanos();

    for (U64 i = 0; i < arrayEntries; i++) {
        if (buffer[i] != i) {
            KFLUSH_AFTER {
                INFO(STRING("arithmetic error at i="));
                INFO(i);
                INFO(STRING(", expected="));
                INFO(i);
                INFO(STRING(", actual="));
                INFO(buffer[i], NEWLINE);
            }
        }
    }

    munmap(buffer, TEST_MEMORY_AMOUNT);

    return (Timing){.cycles = endCycleCount - startCycleCount,
                    .millis = (endNanos - startNanos) / 1000000};
}

static void printOutcomeRealloc(Timing result) {
    KFLUSH_AFTER {
        INFO(STRING("Average clockcycles: "));
        INFO(result.cycles, NEWLINE);
        INFO(STRING("Average time ms: "));
        INFO(result.millis, NEWLINE);
    }
}

static void printOutcomeMapped(Timing result, bool is2MiBPage) {
    KFLUSH_AFTER {
        if (is2MiBPage) {
            INFO(STRING("Page Size: 2097152\n"));
        } else {
            INFO(STRING("Page Size: 4096\n"));
        }
        INFO(STRING("Average clockcycles: "));
        INFO(result.cycles, NEWLINE);
        INFO(STRING("Average time ms: "));
        INFO(result.millis, NEWLINE);
    }
}

static void partialMappedWritingTest(bool is2MiBPage) {
    BiskiState state;
    biskiSeed(&state, PRNG_SEED);

    Timing bestSoFar = {.cycles = 0, .millis = 0};
    for (U64 i = 0; i < TEST_ITERATIONS; i++) {
        Timing timing = runMappingTest(
            RING_RANGE_VALUE(biskiNext(&state), MAX_TEST_ENTRIES), is2MiBPage,
            false);

        bestSoFar.cycles += timing.cycles;
        bestSoFar.millis += timing.millis;
    }

    bestSoFar.cycles /= TEST_ITERATIONS;
    bestSoFar.millis /= TEST_ITERATIONS;

    printOutcomeMapped(bestSoFar, is2MiBPage);
}

static void fullMappedWritingTest(bool is2MiBPage) {
    Timing bestSoFar = {.cycles = 0, .millis = 0};
    for (U64 i = 0; i < TEST_ITERATIONS; i++) {
        Timing timing = runMappingTest(MAX_TEST_ENTRIES, is2MiBPage, false);

        bestSoFar.cycles += timing.cycles;
        bestSoFar.millis += timing.millis;
    }

    bestSoFar.cycles /= TEST_ITERATIONS;
    bestSoFar.millis /= TEST_ITERATIONS;

    printOutcomeMapped(bestSoFar, is2MiBPage);
}

static void fullMappedBaselineWritingTest(bool is2MiBPage) {
    Timing bestSoFar = {.cycles = 0, .millis = 0};
    for (U64 i = 0; i < TEST_ITERATIONS; i++) {
        Timing timing = runMappingTest(MAX_TEST_ENTRIES, is2MiBPage, true);

        bestSoFar.cycles += timing.cycles;
        bestSoFar.millis += timing.millis;
    }

    bestSoFar.cycles /= TEST_ITERATIONS;
    bestSoFar.millis /= TEST_ITERATIONS;

    printOutcomeMapped(bestSoFar, is2MiBPage);
}

static void fullMappingTest() {
    KFLUSH_AFTER { INFO(STRING("Starting full mapping test...\n")); }
    fullMappedWritingTest(false);
    fullMappedWritingTest(true);
    KFLUSH_AFTER { INFO(STRING("Starting full mapping baseline test...\n")); }
    fullMappedBaselineWritingTest(false);
    fullMappedBaselineWritingTest(true);
    KFLUSH_AFTER { INFO(STRING("\n\n")); }
}

static void partialMappingTest() {
    KFLUSH_AFTER { INFO(STRING("Starting partial mapping test...\n")); }
    partialMappedWritingTest(false);
    partialMappedWritingTest(true);
    KFLUSH_AFTER { INFO(STRING("\n\n")); }
}

static void fullReallocWritingTest() {
    KFLUSH_AFTER { INFO(STRING("Starting full realloc writing test...\n")); }
    Timing bestSoFar = {.cycles = 0, .millis = 0};
    for (U64 i = 0; i < TEST_ITERATIONS; i++) {
        U64_d_a array = createDynamicArray();

        U64 startNanos = currentTimeNanos();
        U64 startCycleCount = currentCycleCounter(true, false);

        for (U64 i = 0; i < MAX_TEST_ENTRIES; i++) {
            U64_D_A_APPEND(array, i);
        }

        U64 endCycleCount = currentCycleCounter(false, true);
        U64 endNanos = currentTimeNanos();

        for (U64 i = 0; i < MAX_TEST_ENTRIES; i++) {
            if (array.buf[i] != i) {
                KFLUSH_AFTER {
                    INFO(STRING("arithmetic error at i="));
                    INFO(i);
                    INFO(STRING(", expected="));
                    INFO(i);
                    INFO(STRING(", actual="));
                    INFO(array.buf[i], NEWLINE);
                }
            }
        }

        bestSoFar.cycles += (endCycleCount - startCycleCount);
        bestSoFar.millis += ((endNanos - startNanos) / 1000000);

        free(array.buf);
    }

    bestSoFar.cycles /= TEST_ITERATIONS;
    bestSoFar.millis /= TEST_ITERATIONS;

    printOutcomeRealloc(bestSoFar);
}

static void partialReallocWritingTest() {
    KFLUSH_AFTER { INFO(STRING("Starting partial realloc writing test...\n")); }
    Timing bestSoFar = {.cycles = 0, .millis = 0};

    BiskiState state;
    biskiSeed(&state, PRNG_SEED);
    for (U64 i = 0; i < TEST_ITERATIONS; i++) {
        U64_d_a array = createDynamicArray();
        U64 arrayEntries =
            RING_RANGE_VALUE(biskiNext(&state), MAX_TEST_ENTRIES);

        U64 startNanos = currentTimeNanos();
        U64 startCycleCount = currentCycleCounter(true, false);

        for (U64 i = 0; i < arrayEntries; i++) {
            U64_D_A_APPEND(array, i);
        }

        U64 endCycleCount = currentCycleCounter(false, true);
        U64 endNanos = currentTimeNanos();

        for (U64 i = 0; i < arrayEntries; i++) {
            if (array.buf[i] != i) {
                KFLUSH_AFTER {
                    INFO(STRING("arithmetic error at i="));
                    INFO(i);
                    INFO(STRING(", expected="));
                    INFO(i);
                    INFO(STRING(", actual="));
                    INFO(array.buf[i], NEWLINE);
                }
            }
        }

        bestSoFar.cycles += (endCycleCount - startCycleCount);
        bestSoFar.millis += ((endNanos - startNanos) / 1000000);

        free(array.buf);
    }

    bestSoFar.cycles /= TEST_ITERATIONS;
    bestSoFar.millis /= TEST_ITERATIONS;

    printOutcomeRealloc(bestSoFar);
}

static void fullReallocTest() {
    fullReallocWritingTest();
    partialReallocWritingTest();
    KFLUSH_AFTER { INFO(STRING("\n\n")); }
}

int main() {
    for (U64 i = 0; i < 2; i++) {
        fullMappingTest();
        partialMappingTest();

        fullReallocTest();
    }

    return 0;
}
