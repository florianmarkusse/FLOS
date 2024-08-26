#include "cpu/idt.h"
#include "interoperation/array.h"
#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/memory/descriptor.h"
#include "log/log.h"
#include "memory/management/physical.h"
#include "test-framework/test.h"
#include "text/string.h"
#include "util/macros.h"
#include "util/maths.h"
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define TOTAL_BASE_PAGES (U64)(512 * 512 * 5)
#define MEMORY (PAGE_FRAME_SIZE * TOTAL_BASE_PAGES)

bool compareInterrupts(bool *expectedFaults) {
    bool *actualFaults = getTriggeredFaults();

    for (U64 i = 0; i < FAULT_NUMS; i++) {
        if (expectedFaults[i] != actualFaults[i]) {
            return false;
        }
    }

    return true;
}

void appendInterrupt(Fault fault) {
    LOG(STRING("Fault #: "));
    LOG(fault);
    LOG(STRING("\tMsg: "));
    LOG(stringWithMinSizeDefault(faultToString(fault), 30));
}

void appendExpectedInterrupt(Fault fault) {
    LOG(STRING("Missing interrupt\n"));
    appendInterrupt(fault);
    LOG(STRING("\n"));
}

void appendInterrupts(bool *expectedFaults) {
    bool *actualFaults = getTriggeredFaults();
    LOG(STRING("Interrupts Table\n"));
    for (U64 i = 0; i < FAULT_NUMS; i++) {
        appendInterrupt(i);
        LOG(STRING("\tExpected: "));
        LOG(stringWithMinSizeDefault(
            expectedFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        LOG(STRING("\tActual: "));
        LOG(stringWithMinSizeDefault(
            actualFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        if (expectedFaults[i] != actualFaults[i]) {
            LOG(STRING("\t!!!"));
        }
        LOG(STRING("\n"));
    }
}

#define EXPECT_NO_FAILURE                                                      \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[FAULT_NUMS];                                \
        TEST_FAILURE { appendInterrupts(expectedFaults); }                     \
        break;                                                                 \
    }

#define EXPECT_SINGLE_FAULT(expectedFault)                                     \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[FAULT_NUMS];                                \
        expectedFaults[expectedFault] = true;                                  \
        if (compareInterrupts(expectedFaults)) {                               \
            testSuccess();                                                     \
            break;                                                             \
        } else {                                                               \
            TEST_FAILURE { appendInterrupts(expectedFaults); }                 \
            break;                                                             \
        }                                                                      \
    }

static PhysicalBasePage *memoryStart = NULL;
MemoryDescriptor createDescriptor(MemoryType type, U64 numberOfPages,
                                  U64 *index) {
    U64 indexToUse = *index;
    *index += numberOfPages;
    return (MemoryDescriptor){.type = type,
                              .attribute = 0,
                              .virtualStart = (U64)(memoryStart + indexToUse),
                              .physicalStart = (U64)(memoryStart + indexToUse),
                              .numberOfPages = numberOfPages};
}

int main() {
    testSuiteStart(STRING("Memory Management"));

    void *jmp_buf[5];
    initIDTTest(jmp_buf);

    PhysicalBasePage *pages = mmap(NULL, MEMORY, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pages == MAP_FAILED) {
        FLUSH_AFTER {
            LOG(STRING("Failed to allocate memory!\n"));
            LOG(STRING("Error code: "));
            LOG(errno, NEWLINE);
            LOG(STRING("Error message: "));
            LOG(STRING_LEN(strerror(errno), strlen(strerror(errno))), NEWLINE);
        }
        return -1;
    }

    // Setting the memoryStart to a page that is a power of 1GiB. Otherwise the
    // math to do the tests get very wacky.
    memoryStart = (PhysicalBasePage *)(ALIGN_UP_EXP(
        (U64)pages, PAGE_FRAME_SHIFT + 2 * PAGE_TABLE_SHIFT));

    TEST_TOPIC(STRING("Physical Memory Management")) {
        TEST_TOPIC(STRING("Initing")) {
            TEST(STRING("No available physical memory")) {
                resetTriggeredFaults();

                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(RESERVED_MEMORY_TYPE, 100, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory = {
                    .totalDescriptorSize =
                        sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                    .descriptorSize = sizeof(MemoryDescriptor),
                    .descriptors = descriptors};

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                initPhysicalMemoryManager(kernelMemory);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
            TEST(STRING("Too little available physical memory")) {
                resetTriggeredFaults();
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 2, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory = {
                    .totalDescriptorSize =
                        sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                    .descriptorSize = sizeof(MemoryDescriptor),
                    .descriptors = descriptors};

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                initPhysicalMemoryManager(kernelMemory);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
            TEST(STRING("Single region of 3 memory pages")) {
                resetTriggeredFaults();
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory = {
                    .totalDescriptorSize =
                        sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                    .descriptorSize = sizeof(MemoryDescriptor),
                    .descriptors = descriptors};

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocPhysicalPages(
                    (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                   .len = 1},
                    BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            TEST(STRING("3 regions of single page memory")) {
                resetTriggeredFaults();
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory = {
                    .totalDescriptorSize =
                        sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                    .descriptorSize = sizeof(MemoryDescriptor),
                    .descriptors = descriptors};

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocPhysicalPages(
                    (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                   .len = 1},
                    BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            TEST(STRING("Multiple regions of memory")) {
                resetTriggeredFaults();
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 500, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 520, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PAGE_TABLE_ENTRIES - 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index)};
                KernelMemory kernelMemory = {
                    .totalDescriptorSize =
                        sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                    .descriptorSize = sizeof(MemoryDescriptor),
                    .descriptors = descriptors};

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocPhysicalPages(
                    (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                   .len = 1},
                    LARGE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        TEST_TOPIC(STRING("Incontiguous memory")) {
            TEST(STRING("Single level stealing")) {
                resetTriggeredFaults();
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 500, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 521, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PAGE_TABLE_ENTRIES * 5 + 1, &index)};
                KernelMemory kernelMemory = {
                    .totalDescriptorSize =
                        sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                    .descriptorSize = sizeof(MemoryDescriptor),
                    .descriptors = descriptors};

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 4; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 510; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        BASE_PAGE);
                }

                allocPhysicalPages(
                    (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                   .len = 1},
                    LARGE_PAGE);

                for (U64 i = 0; i < 511; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        BASE_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocPhysicalPages(
                    (FreeMemory_a){
                        .buf = (FreeMemory[]){(FreeMemory){}, (FreeMemory){}},
                        .len = 2},
                    BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            TEST(STRING("Multi-level stealing")) {
                resetTriggeredFaults();
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(
                        CONVENTIONAL_MEMORY,
                        PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES - 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES,
                                     &index)};
                KernelMemory kernelMemory = {
                    .totalDescriptorSize =
                        sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                    .descriptorSize = sizeof(MemoryDescriptor),
                    .descriptors = descriptors};

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 511; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 509; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        BASE_PAGE);
                }

                for (U64 i = 0; i < 100; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        BASE_PAGE);
                }

                for (U64 i = 0; i < 511; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 412; i++) {
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                       .len = 1},
                        BASE_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocPhysicalPages(
                    (FreeMemory_a){
                        .buf = (FreeMemory[]){(FreeMemory){}, (FreeMemory){}},
                        .len = 2},
                    HUGE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        TEST_TOPIC(STRING("Contiguous memory")) {
            TEST(STRING("Not implemented")) {
                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        TEST_TOPIC(STRING("Combined")) {
            TEST(STRING("Not implemented")) {
                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }
    }

    return testSuiteFinish();
}
