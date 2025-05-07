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
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/management.h"
#include "shared/memory/policy.h"
#include "shared/memory/policy/status.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h" // for U32

// void appendDescriptionHeaders(RSDPResult rsdp);

static constexpr auto INIT_MEMORY = (16 * MiB);

static constexpr auto TEST_MEMORY = (64 * MiB);
static void stuff() {
    void *testMemory = (void *)allocAndMap(TEST_MEMORY);
    Arena arena = (Arena){.curFree = testMemory,
                          .beg = testMemory,
                          .end = testMemory + TEST_MEMORY};
    if (setjmp(arena.jmp_buf)) {
        KFLUSH_AFTER { KLOG(STRING("Ran out of test memory capacity\n")); }
        while (1) {
            ;
        }
    }

    U64 totalElements = (TEST_MEMORY / 2) / sizeof(U64);
    U64_a array = {.buf = NEW(&arena, U64, totalElements), .len = 0};

    KFLUSH_AFTER {
        INFO(STRING("Allocated an array of U64 with space for "));
        INFO(totalElements);
        INFO(STRING(" elements.\n"));
        INFO(STRING("Buffer location is: "));
        INFO(array.buf, NEWLINE);
    }

    for (U64 i = 0; i < totalElements; i++) {
        array.buf[i] = i;
    }

    for (U64 i = 1; i < totalElements; i *= 2) {
        KFLUSH_AFTER {
            INFO(STRING("At index "));
            INFO(i);
            INFO(STRING(", value: "));
            INFO(array.buf[i], NEWLINE);
        }
    }

    U64 *virtual = allocVirtualMemory(sizeof(U64) * totalElements,
                                      alignof(U64));

    KFLUSH_AFTER {
        INFO(STRING("Allocated an array of U64 with virtual space for "));
        INFO(totalElements);
        INFO(STRING(" elements.\n"));
        INFO(STRING("Buffer location is: "));
        INFO(virtual, NEWLINE);
    }

    for (U64 i = 0; i < totalElements; i++) {
        virtual[i] = i;
    }
}

__attribute__((section("kernel-start"))) int
kernelmain(PackedKernelParameters *kernelParams) {
    archInit(kernelParams->archInit);

    initMemoryManager(kernelParams->memory);

    void *initMemory = (void *)allocAndMap(INIT_MEMORY);
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

    freeMapped((Memory){.start = (U64)arena.curFree,
                        .bytes = (U64)(arena.end - arena.curFree)});
    freeMapped(
        (Memory){.start = (U64)kernelParams, .bytes = sizeof(*kernelParams)});

    // NOTE: from here, everything is initialized
    KFLUSH_AFTER { KLOG(STRING("ITS WEDNESDAY MY DUDES\n")); }

    KFLUSH_AFTER {
        //
        appendMemoryManagementStatus();
        INFO(STRING("Cycles per microsecond: "));
        INFO(kernelParams->archInit.tscFrequencyPerMicroSecond, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("\n\n")); }

    stuff();

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
