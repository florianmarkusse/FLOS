#include "abstraction/efi.h"

#include "abstraction/log.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/virtual/allocator.h"
#include "abstraction/time.h"
#include "efi-to-kernel/memory/definitions.h"
#include "efi/error.h"
#include "efi/firmware/base.h"   // for PhysicalAddress
#include "efi/firmware/system.h" // for PhysicalAddress
#include "efi/globals.h"
#include "efi/memory/physical.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/configuration/features.h"
#include "x86/efi-to-kernel/params.h"
#include "x86/efi/gdt.h"
#include "x86/gdt.h"
#include "x86/idt.h"
#include "x86/memory/definitions.h"
#include "x86/memory/pat.h"
#include "x86/memory/virtual.h"

void bootstrapProcessorWork(Arena scratch) {
    rootPageTable = getZeroedPageTable();

    KFLUSH_AFTER {
        INFO(STRING("root page table memory location: "));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)rootPageTable, NEWLINE);
    }

    disablePIC();

    gdtData =
        allocateKernelStructure(sizeof(PhysicalBasePage) * 3,
                                alignof(PhysicalBasePage), false, scratch);
    gdtDescriptor = prepNewGDT((PhysicalBasePage *)gdtData);

    // Maybe when there is other CPUs in here??
    //    // NOTE: WHY????
    //    globals.st->boot_services->stall(100000);

    asm volatile("pause" : : : "memory"); // memory barrier
}

// Basic CPUID leafs
static constexpr auto BASIC_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER = 1;
static constexpr auto EXTENDED_FEATURES = 7;

static constexpr auto XSAVE_CPU_SUPPORT = 13;

static constexpr auto XSAVE_ALIGNMENT = 64;

static constexpr auto BASIC_MAX_REQUIRED_PARAMETER =
    BASIC_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER;

// Extended CPUID leafs
static constexpr auto EXTENDED_MAX_VALUE_PARAMETER = 0x80000000;
static constexpr auto EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER =
    EXTENDED_MAX_VALUE_PARAMETER + 1;

static constexpr auto EXTENDED_MAX_REQUIRED_PARAMETER =
    EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER;

static constexpr auto CALIBRATION_MICROSECONDS = 10000;
static U64 calibrateWait() {
    CPUIDResult leaf15 = CPUID(0x15);
    if (leaf15.ebx && leaf15.ecx > 10'000) {
        return (leaf15.ecx * (leaf15.ebx / leaf15.eax)) / 1'000'000;
}
else {
    CPUIDResult leaf16 = CPUID(0x16);
        if (leaf16.eax > 1'000) {
            return leaf16.eax;
}
}

U64 currentCycles = currentCycleCounter(false, false);
globals.st->boot_services->stall(CALIBRATION_MICROSECONDS);
U64 endCycles = currentCycleCounter(false, false);
return (endCycles - currentCycles) / CALIBRATION_MICROSECONDS;
}

ArchParamsRequirements initArchitecture(Arena scratch) {
    asm volatile("cli");

    U32 maxBasicCPUID = CPUID(0x0).eax;
    if (maxBasicCPUID < BASIC_MAX_REQUIRED_PARAMETER) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support required CPUID of "));
            ERROR(BASIC_MAX_REQUIRED_PARAMETER, NEWLINE);
        }
    }

    CPUIDResult processorInfoAndFeatureBits = CPUID(0x1);
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

    if (!features.PAT) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support PAT!")); }
    }
    KFLUSH_AFTER { INFO(STRING("Configuring PAT\n")); }
    CPUConfigurePAT();

    if (!features.SSE) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support SSE!")); }
    }
    CPUEnableSSE();

    if (!features.XSAVE) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support XSAVE!")); }
    }
    if (!features.AVX) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support AVX256!")); }
    }
    bool supportsAVX512 = false;
    CPUIDResult extendedProcessorFeatures = CPUID(EXTENDED_FEATURES);
    if (extendedProcessorFeatures.ebx & (1 << 16)) {
        supportsAVX512 = true;
        KFLUSH_AFTER { INFO(STRING("Support for AVX512 found\n")); }
    } else {
        KFLUSH_AFTER { INFO(STRING("No Support for AVX512 found\n")); }
    }

    KFLUSH_AFTER {
        INFO(STRING("Configuuing XSAVE to enable FPU / SSE / AVX\n"));
    }
    enableAndConfigureXSAVE(supportsAVX512);

    CPUIDResult XSAVECPUSupport = CPUIDWithSubleaf(XSAVE_CPU_SUPPORT, 1);
    if (XSAVECPUSupport.eax & (1 << 0)) {
        KFLUSH_AFTER { INFO(STRING("Support for XSAVEOPT found!\n")); }
    } else {
        EXIT_WITH_MESSAGE { ERROR(STRING("No Support for XSAVEOPT found!\n")); }
    }

    U32 XSAVESize;
    if (XSAVECPUSupport.eax & (1 << 3)) {
        XSAVESize = CPUIDWithSubleaf(XSAVE_CPU_SUPPORT, 2).ebx;
        KFLUSH_AFTER { INFO(STRING("Support for XSAVES found!\n")); }
    } else {
        XSAVESize = CPUIDWithSubleaf(XSAVE_CPU_SUPPORT, 0).ebx;
        KFLUSH_AFTER { INFO(STRING("No Support for XSAVES found!\n")); }
    }
    KFLUSH_AFTER {
        INFO(STRING(
            "Maximum required size in bytes for storing state components "
            "with current components: "));
        INFO(XSAVESize, NEWLINE);
    }

    U8 *XSAVEAddress = (U8 *)allocateKernelStructure(XSAVESize, XSAVE_ALIGNMENT,
                                                     false, scratch);
    memset(XSAVEAddress, 0, XSAVESize);
    INFO(STRING("XSAVE space location: "));
    INFO(XSAVEAddress, NEWLINE);
    XSAVESpace = XSAVEAddress;

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

    KFLUSH_AFTER { INFO(STRING("Bootstrap processor work\n")); }
    bootstrapProcessorWork(scratch);

    return (ArchParamsRequirements){.bytes = sizeof(X86ArchParams),
                                    .align = alignof(X86ArchParams)};
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

void fillArchParams(void *archParams) {
    X86ArchParams *x86ArchParams = (X86ArchParams *)archParams;

    KFLUSH_AFTER { INFO(STRING("Calibrating timer\n")); }
    x86ArchParams->tscFrequencyPerMicroSecond = calibrateWait();

    x86ArchParams->XSAVELocation = XSAVESpace;

    x86ArchParams->rootPageMetaData.children =
        (PackedPageMetaDataNode *)rootPageMetaData.children;
    x86ArchParams->rootPageMetaData.metaData.entriesMapped =
        rootPageMetaData.metaData.entriesMapped;
    x86ArchParams->rootPageMetaData.metaData
        .entriesMappedWithSmallerGranularity =
        rootPageMetaData.metaData.entriesMappedWithSmallerGranularity;
}
