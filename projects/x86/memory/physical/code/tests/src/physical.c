#include "test/physical.h"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#include "efi-to-kernel/kernel-parameters.h"
#include "efi-to-kernel/memory/descriptor.h"
#include "posix/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/definitions.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "x86/fault.h"
#include "x86/fault/print/test.h"
#include "x86/idt/mock.h"
#include "x86/memory/definitions.h"
#include "x86/memory/physical.h"

static constexpr auto TOTAL_PAGES = (U64)(512 * 512 * 5);
static constexpr auto PAGE_SIZE_TO_USE = X86_4KIB_PAGE;
static constexpr auto MEMORY = (PAGE_SIZE_TO_USE * TOTAL_PAGES);

#define WITH_INIT_TEST(testString)                                             \
    resetTriggeredFaults();                                                    \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        TEST_FAILURE {                                                         \
            PLOG(                                                              \
                STRING(                                                        \
                    "Interrupt Jumper was not set up yet! Killing test loop"), \
                NEWLINE);                                                      \
        }                                                                      \
        break;                                                                 \
    }                                                                          \
    TEST(testString)

#define EXPECT_NO_FAILURE                                                      \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[CPU_FAULT_COUNT];                           \
        TEST_FAILURE {                                                         \
            appendInterrupts(expectedFaults, getTriggeredFaults());            \
        }                                                                      \
        break;                                                                 \
    }

#define EXPECT_SINGLE_FAULT(expectedFault)                                     \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[CPU_FAULT_COUNT];                           \
        expectedFaults[expectedFault] = true;                                  \
        if (compareInterrupts(expectedFaults)) {                               \
            testSuccess();                                                     \
            break;                                                             \
        } else {                                                               \
            TEST_FAILURE {                                                     \
                appendInterrupts(expectedFaults, getTriggeredFaults());        \
            }                                                                  \
            break;                                                             \
        }                                                                      \
    }

static PhysicalBasePage *memoryStart = nullptr;
PagedMemory createDescriptor(U64 numberOfPages, U64 *index) {
    U64 indexToUse = *index;

    *index += (numberOfPages * PAGE_SIZE_TO_USE);
    return (PagedMemory){.start = (U64)(memoryStart + indexToUse),
                         .numberOfPages = numberOfPages};
}

KernelMemory createKernelMemory(PagedMemory_a descriptors) {
    return (KernelMemory){
        .memory = descriptors,
        .pages = CEILING_DIV_VALUE(sizeof(PagedMemory) * descriptors.len,
                                   (U64)PAGE_SIZE_TO_USE)};
}

void testPhysicalMemoryManagement() {
    void *jmp_buf[5];
    if (__builtin_setjmp(jmp_buf)) {
        TEST_FAILURE {
            PLOG(STRING("Interrupt Jumper was not set up yet!"), NEWLINE);
        }
        return;
    }

    initIDTTest(jmp_buf);

    PhysicalBasePage *pages = mmap(nullptr, MEMORY, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pages == MAP_FAILED) {
        PFLUSH_AFTER(STDERR) {
            PLOG(STRING("Failed to allocate memory!\n"));
            PLOG(STRING("Error code: "));
            PLOG(errno, NEWLINE);
            PLOG(STRING("Error message: "));
            PLOG(STRING_LEN(strerror(errno), strlen(strerror(errno))), NEWLINE);
        }
        return;
    }

    memoryStart = (PhysicalBasePage *)ALIGN_UP_VALUE(
        (U64)pages, X86_4KIB_PAGE * 2 * PageTableFormat.ENTRIES);

    TEST_TOPIC(STRING("Physical Memory Management")) {
        TEST_TOPIC(STRING("Initing")) {
            WITH_INIT_TEST(STRING("No available physical memory")) {
                U64 index = 0;
                PagedMemory descriptors[] = {
                    createDescriptor(100, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index)};
                KernelMemory kernelMemory = createKernelMemory((PagedMemory_a){
                    .buf = descriptors, .len = COUNTOF(descriptors)});

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                initPhysicalMemoryManager(kernelMemory);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
            WITH_INIT_TEST(STRING("Too little available physical memory")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(2, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                initPhysicalMemoryManager(kernelMemory);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
            WITH_INIT_TEST(STRING("Single region of 3 memory pages")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(3, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    X86_4KIB_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("3 regions of single page memory")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(1, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index), createDescriptor(1, &index),
                    createDescriptor(1, &index), createDescriptor(1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    X86_4KIB_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("Multiple regions of memory")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor( 3, &index),
                    createDescriptor( 500, &index),
                    createDescriptor( 520, &index),
                    createDescriptor(
                                     PageTableFormat.ENTRIES - 1, &index),
                    createDescriptor 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    X86_2MIB_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        TEST_TOPIC(STRING("Incontiguous allocation")) {
            WITH_INIT_TEST(STRING("Single-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(3, &index), createDescriptor(500, &index),
                    createDescriptor(521, &index),
                    createDescriptor(PageTableFormat.ENTRIES * 5 + 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 4; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_2MIB_PAGE);
                }

                for (U64 i = 0; i < 510; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_4KIB_PAGE);
                }

                {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_2MIB_PAGE);
                }

                for (U64 i = 0; i < 511; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_4KIB_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[2];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    X86_4KIB_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("Multi-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(3, &index),
                    createDescriptor(

                        PageTableFormat.ENTRIES * PageTableFormat.ENTRIES - 3,
                        &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PageTableFormat.ENTRIES *
                                         PageTableFormat.ENTRIES,
                                     &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 511; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_2MIB_PAGE);
                }

                for (U64 i = 0; i < 509; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_4KIB_PAGE);
                }

                for (U64 i = 0; i < 100; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_4KIB_PAGE);
                }

                for (U64 i = 0; i < 511; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_2MIB_PAGE);
                }

                for (U64 i = 0; i < 412; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        X86_4KIB_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[2];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    X86_1GIB_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        TEST_TOPIC(STRING("Contiguous allocation")) {
            WITH_INIT_TEST(STRING("Single-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(3, &index), createDescriptor(500, &index),
                    createDescriptor(521, &index),
                    createDescriptor(PageTableFormat.ENTRIES * 5 + 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                allocContiguousPhysicalPages(498, X86_4KIB_PAGE);
                allocContiguousPhysicalPages(9, X86_4KIB_PAGE);
                allocContiguousPhysicalPages(2, X86_4KIB_PAGE);
                allocContiguousPhysicalPages(1, X86_4KIB_PAGE);

                allocContiguousPhysicalPages(500, X86_4KIB_PAGE);
                allocContiguousPhysicalPages(512 * 5, X86_4KIB_PAGE);
                allocContiguousPhysicalPages(12, X86_4KIB_PAGE);

                freePhysicalPage((PagedMemory){.start = 0, .numberOfPages = 1},
                                 X86_4KIB_PAGE);

                allocContiguousPhysicalPages(1, X86_4KIB_PAGE);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocContiguousPhysicalPages(1, X86_4KIB_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("Multi-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(3, &index),
                    createDescriptor(

                        PageTableFormat.ENTRIES * PageTableFormat.ENTRIES - 3,
                        &index),
                    createDescriptor(PageTableFormat.ENTRIES *
                                         PageTableFormat.ENTRIES,
                                     &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                allocContiguousPhysicalPages(511, X86_2MIB_PAGE);
                allocContiguousPhysicalPages(507, X86_4KIB_PAGE);
                allocContiguousPhysicalPages(2, X86_4KIB_PAGE);

                allocContiguousPhysicalPages(PageTableFormat.ENTRIES *
                                                 PageTableFormat.ENTRIES,
                                             X86_4KIB_PAGE);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocContiguousPhysicalPages(1, X86_4KIB_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        WITH_INIT_TEST(
            STRING("Combining contiguous and incontiguous allocation")) {
            U64 index = 0;
            MemoryDescriptor descriptors[] = {
                createDescriptor(3, &index),
                createDescriptor(

                    PageTableFormat.ENTRIES * PageTableFormat.ENTRIES - 3,
                    &index),
                createDescriptor(

                    PageTableFormat.ENTRIES * PageTableFormat.ENTRIES, &index)};
            KernelMemory kernelMemory =
                createKernelMemory(descriptors, COUNTOF(descriptors));

            EXPECT_NO_FAILURE;

            initPhysicalMemoryManager(kernelMemory);

            allocContiguousPhysicalPages(511, X86_2MIB_PAGE);

            for (U64 i = 0; i < 64; i++) {
                PagedMemory memoryForAddresses[8];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    X86_2MIB_PAGE);
            }

            for (U64 i = 0; i < 509; i++) {
                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses, .len = 1},
                    X86_4KIB_PAGE);
            }

            freePhysicalPage((PagedMemory){.start = 0, .numberOfPages = 100},
                             X86_4KIB_PAGE);

            for (U64 i = 0; i < 100; i++) {
                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses, .len = 1},
                    X86_4KIB_PAGE);
            }

            PagedMemory freePages[] = {
                (PagedMemory){.start = 0, .numberOfPages = 1},
                (PagedMemory){.start = 0, .numberOfPages = 2},
                (PagedMemory){.start = 0, .numberOfPages = 3},
                (PagedMemory){.start = 0, .numberOfPages = 4},
                (PagedMemory){.start = 0, .numberOfPages = 5},
                (PagedMemory){.start = 0, .numberOfPages = 6}};
            freePhysicalPages(
                (PagedMemory_a){.buf = freePages, .len = COUNTOF(freePages)},
                X86_1GIB_PAGE);

            for (U64 i = 0; i < 21; i++) {
                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses, .len = 512},
                    X86_2MIB_PAGE);
            }

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            allocContiguousPhysicalPages(1, X86_4KIB_PAGE);

            TEST_FAILURE {
                appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
            }
        }
    }
}
