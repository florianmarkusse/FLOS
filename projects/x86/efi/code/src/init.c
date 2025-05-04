#include "abstraction/efi.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/base.h"   // for PhysicalAddress
#include "efi/firmware/system.h" // for PhysicalAddress
#include "efi/globals.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/virtual.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/configuration/features.h"
#include "x86/efi/gdt.h"
#include "x86/gdt.h"
#include "x86/memory/definitions.h"
#include "x86/memory/pat.h"
#include "x86/memory/virtual.h"

void bootstrapProcessorWork(Arena scratch) {
    U64 newCR3 = getPageForMappingVirtualMemory(VIRTUAL_MEMORY_MAPPING_SIZE);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)newCR3, 0, X86_4KIB_PAGE);

    rootPageTable = (VirtualPageTable *)newCR3;

    KFLUSH_AFTER {
        INFO(STRING("root page table memory location:"));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)rootPageTable, NEWLINE);
    }

    disablePIC();

    gdtData =
        allocateKernelStructure(sizeof(PhysicalBasePage) * 3,
                                alignof(PhysicalBasePage), false, scratch);
    gdtDescriptor = prepNewGDT((PhysicalBasePage *)gdtData);

    // NOTE: WHY????
    globals.st->boot_services->stall(100000);

    asm volatile("pause" : : : "memory"); // memory barrier
}

// NOTE: this should be done per core probably?
static constexpr auto CALIBRATION_MICROSECONDS = 100;
static void calibrateWait() {
    U32 edx;
    U32 eax;
    asm volatile("rdtscp" : "=a"(eax), "=d"(edx));
    U64 currentCycles = ((U64)edx << 32) | eax;
    globals.st->boot_services->stall(CALIBRATION_MICROSECONDS);
    asm volatile("rdtscp" : "=a"(eax), "=d"(edx));
    U64 endCycles = ((U64)edx << 32) | eax;
    cyclesPerMicroSecond = endCycles - currentCycles / CALIBRATION_MICROSECONDS;
}

// Basic CPUID leafs
static constexpr auto BASIC_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER = 1;

static constexpr auto BASIC_MAX_REQUIRED_PARAMETER =
    BASIC_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER;

// Extended CPUID leafs
static constexpr auto EXTENDED_MAX_VALUE_PARAMETER = 0x80000000;
static constexpr auto EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER =
    EXTENDED_MAX_VALUE_PARAMETER + 1;

static constexpr auto EXTENDED_MAX_REQUIRED_PARAMETER =
    EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER;

void initArchitecture(Arena scratch) {
    asm volatile("cli");

    U32 maxBasicCPUID = CPUID(0).eax;
    if (maxBasicCPUID < BASIC_MAX_REQUIRED_PARAMETER) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support required CPUID of "));
            ERROR(BASIC_MAX_REQUIRED_PARAMETER, NEWLINE);
        }
    }

    CPUIDResult processorInfoAndFeatureBits = CPUID(1);
    features.ecx = processorInfoAndFeatureBits.ecx;
    features.edx = processorInfoAndFeatureBits.edx;
    if (!features.APIC) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support APIC")); }
    }

    U32 BSPID = processorInfoAndFeatureBits.ebx >> 24;

    if (!features.TSC) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support Time Stamp Counter"));
        }
    }
    KFLUSH_AFTER { INFO(STRING("Calibrating timer\n")); }
    calibrateWait();

    if (!features.PGE) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support global memory paging!"));
        }
    }
    KFLUSH_AFTER { INFO(STRING("Enabling PGE\n")); }
    CPUEnablePGE();

    if (!features.FPU) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support FPU!")); }
    }
    KFLUSH_AFTER { INFO(STRING("Enabling FPU\n")); }
    CPUEnableFPU();

    if (!features.SSE) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support SSE!")); }
    }
    KFLUSH_AFTER {
        INFO(STRING(
            "Enabling SSE... even though it doesnt work yet anyway lol\n"));
    }
    CPUEnableSSE();

    if (!features.PAT) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support PAT!")); }
    }
    KFLUSH_AFTER { INFO(STRING("Configuring PAT\n")); }
    CPUConfigurePAT();

    U32 maxExtendedCPUID = CPUID(EXTENDED_MAX_VALUE_PARAMETER).eax;
    if (maxExtendedCPUID < EXTENDED_MAX_REQUIRED_PARAMETER) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support extended CPUID of "));
            ERROR(EXTENDED_MAX_REQUIRED_PARAMETER, NEWLINE);
        }
    }

    CPUIDResult extendedProcessorInfoAndFeatureBits =
        CPUID(EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER);
    if (!(extendedProcessorInfoAndFeatureBits.edx & (1 << 26))) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support Huge Pages!")); }
    }

    //   if (!features.XSAVE) {
    //       messageAndExit(ERROR(STRING("CPU does not support XSAVE!")));
    //   }
    //   KFLUSH_AFTER { INFO(STRING("Enabling XSAVE\n")); }
    //   CPUEnableXSAVE();

    //   if (!features.AVX) {
    //       messageAndExit(ERROR(STRING("CPU does not support AVX!")));
    //   }
    //   KFLUSH_AFTER { INFO(STRING("Enabling AVX\n")); }
    //   CPUEnableAVX();

    KFLUSH_AFTER { INFO(STRING("Bootstrap processor work\n")); }
    bootstrapProcessorWork(scratch);
}

void initVirtualMemory(U64 startingAddress, U64 endingAddress,
                       PackedMemoryTree *virtualMemoryTree, Arena scratch) {
    Arena treeAllocator =
        createAllocatorForMemoryTree(X86_MAX_VIRTUAL_MEMORY_REGIONS, scratch);

    RedBlackNodeMM *root = nullptr;

    RedBlackNodeMM *node = NEW(&treeAllocator, RedBlackNodeMM);
    node->memory = (Memory){.start = startingAddress,
                            .bytes = LOWER_HALF_END - startingAddress};
    insertRedBlackNodeMM(&root, node);

    node = NEW(&treeAllocator, RedBlackNodeMM);
    node->memory = (Memory){.start = HIGHER_HALF_START,
                            .bytes = endingAddress - HIGHER_HALF_START};
    insertRedBlackNodeMM(&root, node);

    *virtualMemoryTree = (PackedMemoryTree){
        .allocator = (PackedArena){.beg = treeAllocator.beg,
                                   .curFree = treeAllocator.curFree,
                                   .end = treeAllocator.end},
        .tree = root};
}
