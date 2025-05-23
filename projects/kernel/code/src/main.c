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

static void stuff() {
    U8 *firstVirtual;
    firstVirtual = allocateMappableMemory(4097, 1);
    KFLUSH_AFTER {
        INFO(STRING("Address received is: "));
        INFO(firstVirtual, NEWLINE);
    }

    U8 *thirdVirtual;
    thirdVirtual = allocateMappableMemory(1 * GiB, 1);
    KFLUSH_AFTER {
        INFO(STRING("Address received is: "));
        INFO(thirdVirtual, NEWLINE);
    }

    U64 previousMemory = 0;
    U64 availableMemory = getAvailablePhysicalMemory();
    KFLUSH_AFTER {
        INFO(STRING("Current available physical memory: "));
        INFO(availableMemory, NEWLINE);
    }

    thirdVirtual[0] = 5;
    thirdVirtual[4096] = 5;
    thirdVirtual[100 * KiB] = 5;
    thirdVirtual[(1 * GiB) - 1] = 5;

    U8 *secondVirtual;
    secondVirtual = allocateMappableMemory(4097, 1);
    KFLUSH_AFTER {
        INFO(STRING("Address received is: "));
        INFO(secondVirtual, NEWLINE);
    }

    firstVirtual[0] = 5;
    firstVirtual[4096] = 6;
    secondVirtual[0] = 5;
    secondVirtual[4096] = 6;

    previousMemory = availableMemory;
    availableMemory = getAvailablePhysicalMemory();
    KFLUSH_AFTER {
        INFO(STRING("Previous available physical memory: "));
        INFO(previousMemory, NEWLINE);
        INFO(STRING("Current available physical memory: "));
        INFO(availableMemory, NEWLINE);
        INFO(STRING("Delta physical memory: "));
        INFO((I64)availableMemory - (I64)previousMemory, NEWLINE);
    }

    freeMappableMemory((Memory){.start = (U64)firstVirtual, .bytes = 4097});
    freeMappableMemory((Memory){.start = (U64)secondVirtual, .bytes = 4097});

    previousMemory = availableMemory;
    availableMemory = getAvailablePhysicalMemory();
    KFLUSH_AFTER {
        INFO(STRING("Previous available physical memory: "));
        INFO(previousMemory, NEWLINE);
        INFO(STRING("Current available physical memory: "));
        INFO(availableMemory, NEWLINE);
        INFO(STRING("Delta physical memory: "));
        INFO((I64)availableMemory - (I64)previousMemory, NEWLINE);
    }
}

__attribute__((section("kernel-start"))) int
kernelmain(PackedKernelParameters *kernelParams) {
    archInit(kernelParams->archParams);
    initMemoryManager(kernelParams->memory);

    void *initMemory = (void *)allocateIdentityMemory(INIT_MEMORY);
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
    KFLUSH_AFTER { KLOG(STRING("ITS WEDNESDAY MY DUDES\n")); }

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
