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
#include "shared/maths/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/management.h"
#include "shared/memory/policy.h"
#include "shared/memory/policy/status.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h" // for U32

// void appendDescriptionHeaders(RSDPResult rsdp);

static constexpr auto INIT_MEMORY = (16 * MiB);

static constexpr auto TEST_MEMORY_AMOUNT = 32 * MiB;
static constexpr auto TEST_ENTRIES = TEST_MEMORY_AMOUNT / (sizeof(U64));

static bool mappingTest(U64 alignment) {
    U64 iterations = 1;
    U64 sum = 0;

    KFLUSH_AFTER {
        INFO(STRING("Running test with alignment of "));
        INFO(alignment, NEWLINE);
    }

    for (U64 iteration = 0; iteration < iterations; iteration++) {
        U64 startPhysicalMemory = getAvailablePhysicalMemory();

        // U64 *virtual = allocateIdentityMemory(TEST_MEMORY_AMOUNT, alignment);
        U64 *virtual = allocateMappableMemory(TEST_MEMORY_AMOUNT, alignment);
        U64 beforePageFaults = getPageFaults();

        U64 startCycleCount = currentCycleCounter(true, false);
        for (U64 i = 0; i < TEST_ENTRIES; i++) {
            virtual[i] = i;
        }
        U64 endCycleCount = currentCycleCounter(false, true);

        for (U64 i = 0; i < TEST_ENTRIES; i++) {
            if (virtual[i] != i) {
                KFLUSH_AFTER {
                    INFO(STRING("arithmetic error at i="));
                    INFO(i);
                    INFO(STRING(", expected="));
                    INFO(i);
                    INFO(STRING(", actual="));
                    INFO(virtual[i], NEWLINE);
                }
                return false;
            }
        }

        U64 pageFaults = getPageFaults();

        // freeIdentityMemory(
        //     (Memory){.start = (U64)virtual, .bytes = TEST_MEMORY_AMOUNT});
        freeMappableMemory(
            (Memory){.start = (U64)virtual, .bytes = TEST_MEMORY_AMOUNT});

        U64 endPhysicalMemory = getAvailablePhysicalMemory();
        if (endPhysicalMemory != startPhysicalMemory) {
            KFLUSH_AFTER {
                INFO(STRING(
                    "Difference in physical memory detected! Started with "));
                INFO(startPhysicalMemory);
                INFO(STRING(" and the delta is "));
                INFO((I64)endPhysicalMemory - (I64)startPhysicalMemory,
                     NEWLINE);
            }
            return false;
        }

        if ((beforePageFaults + (TEST_MEMORY_AMOUNT / alignment)) !=
            pageFaults) {
            KFLUSH_AFTER { INFO(STRING("Incorrect number of page faults.\n")); }
            return false;
        }

        U64 nowPageFaults = getPageFaults();
        if (pageFaults != nowPageFaults) {
            KFLUSH_AFTER { INFO(STRING("Somehow had more page faults\n")); }
            return false;
        }

        sum += (endCycleCount - startCycleCount);
    }

    KFLUSH_AFTER {
        INFO(STRING("Total iterations: "));
        INFO(iterations, NEWLINE);
        INFO(STRING("Average clockcycles: "));
        INFO(sum / iterations, NEWLINE);
    }

    return true;
}

static void mappingTests() {
    // test
    pageSizeToMap = 4 * KiB;
    if (!mappingTest(4 * KiB)) {
        return;
    }
    pageSizeToMap = 64 * KiB;
    if (!mappingTest(64 * KiB)) {
        return;
    }
    pageSizeToMap = 2 * MiB;
    if (!mappingTest(2 * MiB)) {
        return;
    }
}

void test_xsave() {
    U8 xsave_area[4096] __attribute__((aligned(64))) = {0};

    // Attempt to execute XSAVE storing to xsave_area
    asm volatile("xor %%eax, %%eax\n\t" // clear EAX
                 "xor %%edx, %%edx\n\t" // clear EDX
                 "xsave (%0)"
                 :
                 : "r"(xsave_area)
                 : "memory", "rax", "rdx");
}

__attribute__((section("kernel-start"))) int
kernelmain(PackedKernelParameters *kernelParams) {
    archInit(kernelParams->archParams);
    initMemoryManager(kernelParams->memory);

    void *initMemory = (void *)allocateIdentityMemory(INIT_MEMORY, 1);
    Arena arena = (Arena){.curFree = initMemory,
                          .beg = initMemory,
                          .end = initMemory + INIT_MEMORY};
    if (setjmp(arena.jmp_buf)) {
        KFLUSH_AFTER { KLOG(STRING("Ran out of init memory capacity\n")); }
        while (1) {
            ;
        }
    }

    initLogger(&arena);
    initScreen(kernelParams->window, &arena);

    freeIdentityMemory((Memory){.start = (U64)arena.curFree,
                                .bytes = (U64)(arena.end - arena.curFree)});
    freeIdentityMemory(
        (Memory){.start = (U64)kernelParams, .bytes = sizeof(*kernelParams)});

    // NOTE: from here, everything is initialized
    KFLUSH_AFTER {
        KLOG(STRING("ITS WEDNESDAY MY DUDES\n"));
        KLOG(STRING("Available phys memory: "));
        KLOG(getAvailablePhysicalMemory(), NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("\n\n")); }

    KFLUSH_AFTER { INFO(STRING("Testing SSE\n")); }
    asm volatile("movaps %%xmm1, %%xmm0\n" : : : "xmm0", "xmm1");
    KFLUSH_AFTER { INFO(STRING("SSE complete\n")); }

    KFLUSH_AFTER { INFO(STRING("Testing AVX256\n")); }
    asm volatile("vmovaps %%ymm1, %%ymm0\n" : : : "ymm0", "ymm1");
    KFLUSH_AFTER { INFO(STRING("AVX256 complete\n")); }

    KFLUSH_AFTER { INFO(STRING("Testing XSAVE\n")); }
    test_xsave();
    KFLUSH_AFTER { INFO(STRING("XSAVEE test complete\n")); }

    mappingTests();

    KFLUSH_AFTER {
        KLOG(STRING("TESTING IS OVER MY DUDES\n"));
        KLOG(STRING("Available phys memory: "));
        KLOG(getAvailablePhysicalMemory(), NEWLINE);
    }

    while (1) {
        ;
    }
}

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
