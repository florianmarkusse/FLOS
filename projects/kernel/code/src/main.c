#include "abstraction/interrupts.h" // for setupIDT
#include "abstraction/kernel.h"
#include "abstraction/log.h" // for LOG, LOG_CHOOSER_IMPL_1, rewind, pro...
#include "abstraction/memory/virtual/map.h"
#include "abstraction/time.h"
#include "efi-to-kernel/kernel-parameters.h"  // for KernelParameters
#include "efi-to-kernel/memory/definitions.h" // for KERNEL_PARAMS_START
#include "freestanding/log/init.h"
#include "freestanding/peripheral/screen.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/status.h"
#include "shared/memory/policy.h"
#include "shared/memory/policy/status.h"
#include "shared/memory/sizes.h"
#include "shared/prng/biski.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h" // for U32

static constexpr auto INIT_MEMORY = (16 * MiB);

static constexpr auto TEST_MEMORY_AMOUNT = 32 * MiB;
static constexpr auto MAX_TEST_ENTRIES = TEST_MEMORY_AMOUNT / (sizeof(U64));

static constexpr U64 PRNG_SEED = 15466503514872390148ULL;

static U64 TEST_ITERATIONS = 32;

static void appendMemoryDeltaType(AvailableMemoryState startMemory,
                                  AvailableMemoryState endMemory) {
    if (endMemory.memory != startMemory.memory ||
        endMemory.nodes != startMemory.nodes) {
        INFO(STRING(
            "Difference in available memory detected!\n[BEGIN]\tmemory: "));
        INFO(startMemory.memory);
        INFO(STRING(" nodes: "));
        INFO(startMemory.nodes, NEWLINE);
        INFO(STRING("[CURRENT]\tmemory: "));
        INFO(endMemory.memory);
        INFO(STRING(" nodes: "));
        INFO(endMemory.nodes, NEWLINE);
        INFO(STRING("[DELTA]\tmemory: "));
        INFO((I64)endMemory.memory - (I64)startMemory.memory);
        INFO(STRING(" nodes: "));
        INFO((I64)endMemory.nodes - (I64)startMemory.nodes, NEWLINE);
    }
}

static void appendMemoryDelta(AvailableMemoryState startPhysicalMemory,
                              AvailableMemoryState startVirtualMemory) {
    AvailableMemoryState endPhysicalMemory = getAvailablePhysicalMemory();
    appendMemoryDeltaType(startPhysicalMemory, endPhysicalMemory);

    AvailableMemoryState endVirtualMemory = getAvailableVirtualMemory();
    appendMemoryDeltaType(startVirtualMemory, endVirtualMemory);
}

static constexpr auto GROWTH_RATE = 2;
static constexpr auto START_ENTRIES_COUNT = 8;

typedef enum { IDENTITY_MEMORY, MAPPABLE_MEMORY } MemoryWritableType;

static U64 arrayWritingTest(U64 alignment, U64 arrayEntries,
                            MemoryWritableType memoryWritableType,
                            U64 expectedPageFaults) {
    AvailableMemoryState startPhysicalMemory = getAvailablePhysicalMemory();
    AvailableMemoryState startVirtualMemory = getAvailableVirtualMemory();
    U64 beforePageFaults = getPageFaults();

    U64 *buffer;
    U64 cycles;
    if (memoryWritableType == IDENTITY_MEMORY) {
        buffer = allocateIdentityMemory(START_ENTRIES_COUNT * sizeof(U64),
                                        alignment);

        U64_max_a dynamicArray = {
            .buf = buffer, .len = 0, .cap = START_ENTRIES_COUNT};

        U64 startCycleCount = currentCycleCounter(true, false);
        for (U64 i = 0; i < arrayEntries; i++) {
            if (dynamicArray.len >= dynamicArray.cap) {
                U64 currentBytes = dynamicArray.cap * sizeof(U64);
                U64 *temp = allocateIdentityMemory(currentBytes * GROWTH_RATE,
                                                   alignof(U64));
                memcpy(temp, dynamicArray.buf, currentBytes);

                freeIdentityMemory((Memory){.start = (U64)dynamicArray.buf,
                                            .bytes = currentBytes});
                dynamicArray.buf = temp;
                dynamicArray.cap *= GROWTH_RATE;
            }
            dynamicArray.buf[dynamicArray.len] = i;
            dynamicArray.len++;
        }
        U64 endCycleCount = currentCycleCounter(false, true);

        buffer = dynamicArray.buf;

        cycles = endCycleCount - startCycleCount;
    } else {
        buffer = allocateMappableMemory(TEST_MEMORY_AMOUNT, alignment);

        U64 startCycleCount = currentCycleCounter(true, false);

        for (U64 i = 0; i < arrayEntries; i++) {
            buffer[i] = i;
        }
        U64 endCycleCount = currentCycleCounter(false, true);

        cycles = endCycleCount - startCycleCount;
    }

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
            return 0;
        }
    }

    U64 afterPageFaults = getPageFaults();

    if (memoryWritableType == IDENTITY_MEMORY) {
        freeIdentityMemory(
            (Memory){.start = (U64)buffer,
                     .bytes = ceilingPowerOf2(arrayEntries * alignof(U64))});
    } else {
        freeMappableMemory(
            (Memory){.start = (U64)buffer, .bytes = TEST_MEMORY_AMOUNT});
    }

    KFLUSH_AFTER { appendMemoryDelta(startPhysicalMemory, startVirtualMemory); }

    U64 expectedAfterPageFaults = beforePageFaults + expectedPageFaults;
    if (expectedAfterPageFaults != afterPageFaults) {
        KFLUSH_AFTER {
            INFO(STRING("Incorrect number of page faults.\n"));
            INFO(STRING("Expected: "));
            INFO(expectedAfterPageFaults, NEWLINE);
            INFO(STRING("Actual: "));
            INFO(afterPageFaults, NEWLINE);
        }
        return 0;
    }

    return cycles;
}

static bool partialMappingTest(U64 alignment) {
    U64 sum = 0;

    KFLUSH_AFTER {
        INFO(STRING("Page Size: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(alignment), 10));
    }

    BiskiState state;
    biskiSeed(&state, PRNG_SEED);

    for (U64 iteration = 0; iteration < TEST_ITERATIONS; iteration++) {
        U64 entriesToWrite =
            RING_RANGE_VALUE(biskiNext(&state), MAX_TEST_ENTRIES);
        U64 cycles = arrayWritingTest(
            alignment, entriesToWrite, MAPPABLE_MEMORY,
            CEILING_DIV_VALUE((entriesToWrite * sizeof(U64)), alignment));
        if (!cycles) {
            return false;
        }
        sum += cycles;
    }

    KFLUSH_AFTER {
        INFO(STRING("\taverage clockcycles: "));
        INFO(sum / TEST_ITERATIONS, NEWLINE);
    }

    return true;
}

static bool fullMappingTest(U64 alignment) {
    U64 sum = 0;

    KFLUSH_AFTER {
        INFO(STRING("Page Size: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(alignment), 10));
    }

    for (U64 iteration = 0; iteration < TEST_ITERATIONS; iteration++) {
        U64 cycles = arrayWritingTest(
            alignment, MAX_TEST_ENTRIES, MAPPABLE_MEMORY,
            CEILING_DIV_VALUE((MAX_TEST_ENTRIES * sizeof(U64)), alignment));
        if (!cycles) {
            return false;
        }
        sum += cycles;
    }

    KFLUSH_AFTER {
        INFO(STRING("\taverage clockcycles: "));
        INFO(sum / TEST_ITERATIONS, NEWLINE);
    }

    return true;
}

static void identityTests() {
    KFLUSH_AFTER {
        INFO(STRING("Starting identity tests with\n\n"));
        INFO(STRING("Starting full writing test...\n"));
    }

    U64 sum = 0;

    for (U64 iteration = 0; iteration < TEST_ITERATIONS; iteration++) {
        U64 cycles = arrayWritingTest(alignof(U64), MAX_TEST_ENTRIES,
                                      IDENTITY_MEMORY, 0);
        if (!cycles) {
            return;
        }
        sum += cycles;
    }

    KFLUSH_AFTER {
        INFO(STRING("\t\t\t\t\t\taverage clockcycles: "));
        INFO(sum / TEST_ITERATIONS, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("\nStarting partial writing test...\n")); }

    sum = 0;

    BiskiState state;
    biskiSeed(&state, PRNG_SEED);

    for (U64 iteration = 0; iteration < TEST_ITERATIONS; iteration++) {
        U64 entriesToWrite =
            RING_RANGE_VALUE(biskiNext(&state), MAX_TEST_ENTRIES);
        U64 cycles =
            arrayWritingTest(alignof(U64), entriesToWrite, IDENTITY_MEMORY, 0);
        if (!cycles) {
            return;
        }
        sum += cycles;
    }

    KFLUSH_AFTER {
        INFO(STRING("\t\t\t\t\t\taverage clockcycles: "));
        INFO(sum / TEST_ITERATIONS, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("\n")); }
}

static void baselineTest() {
    KFLUSH_AFTER {
        INFO(STRING("Starting baseline test...\n"));
        INFO(STRING("full writing test...\n"));
    }

    U64 sum = 0;

    for (U64 iteration = 0; iteration < TEST_ITERATIONS; iteration++) {
        U64 *buffer = allocateIdentityMemory(MAX_TEST_ENTRIES * sizeof(U64),
                                             alignof(U64));

        U64 startCycleCount = currentCycleCounter(true, false);

        for (U64 i = 0; i < MAX_TEST_ENTRIES; i++) {
            buffer[i] = i;
        }
        U64 endCycleCount = currentCycleCounter(false, true);

        freeIdentityMemory((Memory){.start = (U64)buffer,
                                    .bytes = MAX_TEST_ENTRIES * sizeof(U64)});

        U64 cycles = endCycleCount - startCycleCount;
        sum += cycles;
    }

    KFLUSH_AFTER {
        INFO(STRING("\t\t\t\t\t\taverage clockcycles: "));
        INFO(sum / TEST_ITERATIONS, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("partial writing test...\n")); }

    sum = 0;

    BiskiState state;
    biskiSeed(&state, PRNG_SEED);

    for (U64 iteration = 0; iteration < TEST_ITERATIONS; iteration++) {
        U64 *buffer = allocateIdentityMemory(MAX_TEST_ENTRIES * sizeof(U64),
                                             alignof(U64));
        U64 entriesToWrite =
            RING_RANGE_VALUE(biskiNext(&state), MAX_TEST_ENTRIES);

        U64 startCycleCount = currentCycleCounter(true, false);

        for (U64 i = 0; i < entriesToWrite; i++) {
            buffer[i] = i;
        }
        U64 endCycleCount = currentCycleCounter(false, true);

        freeIdentityMemory((Memory){.start = (U64)buffer,
                                    .bytes = MAX_TEST_ENTRIES * sizeof(U64)});

        U64 cycles = endCycleCount - startCycleCount;
        sum += cycles;
    }

    KFLUSH_AFTER {
        INFO(STRING("\t\t\t\t\t\taverage clockcycles: "));
        INFO(sum / TEST_ITERATIONS, NEWLINE);
    }
}

static void mappingTests() {
    KFLUSH_AFTER {
        INFO(STRING("Starting mapping tests\n\n"));
        INFO(STRING("Starting full mapping test...\n"));
    }

    for (U64 pageSize = 4 * KiB; pageSize <= (2 * MiB); pageSize <<= 1) {
        pageSizeToMap = pageSize;
        if (!fullMappingTest(pageSize)) {
            return;
        }
    }

    KFLUSH_AFTER { INFO(STRING("\nStarting partial mapping test...\n")); }

    for (U64 pageSize = 4 * KiB; pageSize <= (2 * MiB); pageSize <<= 1) {
        pageSizeToMap = pageSize;
        if (!partialMappingTest(pageSize)) {
            return;
        }
    }

    KFLUSH_AFTER { INFO(STRING("\n")); }
}

__attribute__((section("kernel-start"))) int
kernelMain(PackedKernelParameters *kernelParams) {
    archInit(kernelParams->archParams);
    initMemoryManager(&kernelParams->memory);

    void *initMemory = (void *)allocateIdentityMemory(INIT_MEMORY, 1);
    Arena arena = (Arena){.curFree = initMemory,
                          .beg = initMemory,
                          .end = initMemory + INIT_MEMORY};
    if (setjmp(arena.jmpBuf)) {
        KFLUSH_AFTER { KLOG(STRING("Ran out of init memory capacity\n")); }
        while (1) {
            ;
        }
    }

    initLogger(&arena);
    initScreen(&kernelParams->window, &arena);

    freeIdentityMemory((Memory){.start = (U64)arena.curFree,
                                .bytes = (U64)(arena.end - arena.curFree)});
    freeIdentityMemory(
        (Memory){.start = (U64)kernelParams, .bytes = sizeof(*kernelParams)});

    // NOTE: from here, everything is initialized

    KFLUSH_AFTER {
        KLOG(STRING("ITS WEDNESDAY MY DUDES\n"));
        appendMemoryManagementStatus();
    }

    KFLUSH_AFTER { INFO(STRING("\n\n")); }

    for (U64 i = 0; i < 2; i++) {
        BiskiState state;
        biskiSeed(&state, PRNG_SEED);

        KFLUSH_AFTER {
            INFO(STRING("Iterations per test: "));
            INFO(TEST_ITERATIONS, NEWLINE);
            INFO(STRING("Max memory per test: "));
            INFO((U64)TEST_MEMORY_AMOUNT, NEWLINE);
            INFO(STRING("Max array entries used: "));
            INFO((U64)MAX_TEST_ENTRIES, NEWLINE);
            INFO(STRING("Random array entries used: "));
            for (U64 i = 0; i < TEST_ITERATIONS; i++) {
                INFO(
                    (U64)RING_RANGE_VALUE(biskiNext(&state), MAX_TEST_ENTRIES));
                INFO(STRING(" "));
            }
            INFO(STRING("\n"));
        }

        mappingTests();
        identityTests();
        baselineTest();
    }

    KFLUSH_AFTER {
        KLOG(STRING("TESTING IS OVER MY DUDES\n"));
        appendMemoryManagementStatus();
    }

    while (1) {
        ;
    }
}

// void appendDescriptionHeaders(RSDPResult rsdp);

// typedef enum { RSDT, XSDT, NUM_DESCRIPTION_TABLES }
// DescriptionTableVersion; static USize entrySizes[NUM_DESCRIPTION_TABLES]
// = {
//     sizeof(U32),
//     sizeof(U64),
// };
//
// void appendDescriptionHeaders(RSDPResult rsdp) {
//     CAcpiSDT *sdt = nullptr;
//     USize entrySize = 0;
//
//     switch (rsdp.revision) {
//     case RSDP_REVISION_1: {
//         sdt = (CAcpiSDT *)rsdp.rsdp->v1.rsdt_addr;
//         entrySize = entrySizes[RSDT];
//         break;
//     }
//     case RSDP_REVISION_2: {
//         sdt = (CAcpiSDT *)rsdp.rsdp->v2.xsdt_addr;
//         entrySize = entrySizes[XSDT];
//         break;
//     }
//     }
//
//     I8 **descriptionHeaders = (I8 **)&sdt->descriptionHeaders;
//     for (U64 i = 0; i < sdt->header.length - sizeof(CAcpiSDT);
//          i += entrySize) {
//         CAcpiDescriptionTableHeader *header = nullptr;
//         memcpy(&header, descriptionHeaders, entrySize);
//
//         LOG(STRING_LEN(header->signature,
//         ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN),
//             NEWLINE);
//
//         ACPITable tableType =
//             ACPITablesToEnum(STRING_LEN(header->signature, 4));
//
//         switch (tableType) {
//         case MULTIPLE_APIC_DESCRIPTION_TABLE: {
//             MADT *madt = (MADT *)header;
//
//             LOG(STRING("printing the structures of the MADT:"), NEWLINE);
//
//             InterruptControllerStructure *interruptStructures =
//                 madt->interruptStructures;
//             for (U64 j = 0;
//                  j < madt->madt.header.length - sizeof(ConstantMADT);
//                  j += interruptStructures->totalLength) {
//                 LOG(STRING("Type: "));
//                 LOG(interruptStructures->type);
//                 LOG(STRING("Length: "));
//                 LOG(interruptStructures->totalLength, NEWLINE);
//
//                 interruptStructures = (InterruptControllerStructure
//                                            *)((I8 *)interruptStructures +
//                                               interruptStructures->totalLength);
//             }
//
//             break;
//         }
//         default: {
//             LOG(STRING("Did not implement anything for this yet"),
//             NEWLINE);
//         }
//         }
//
//         descriptionHeaders = (I8 **)((I8 *)descriptionHeaders +
//         entrySize);
//     }
// }
