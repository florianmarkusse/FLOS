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
#include "shared/maths.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/configuration/features.h"
#include "x86/efi-to-kernel/params.h"
#include "x86/efi/gdt.h"
#include "x86/gdt.h"
#include "x86/memory/definitions.h"
#include "x86/memory/virtual.h"

static constexpr auto INTERRUPT_STACK_SIZE = 1 * KiB;
static constexpr auto MAX_BYTES_GDT = 64 * KiB;
// the GDT can contain up to 8192 descriptors 8-byte = 655365 bytes = 64 KiB
// - null              = 8 bytes
// - kernel code       = 8 bytes
// - kernel data       = 8 butes
// - padding           = 8 butes
// - x tss descriptors = 16 * x bytes !!! should be aligned to 16 bytes for
// performance reasons.
// So, we align the GDT to 16 bytes, so everything is at least self-aligned.
static void prepareDescriptors(U64 numberOfProcessors, U16 cacheLineSizeBytes,
                               Arena scratch) {
    U64 requiredBytesForDescriptorTable =
        CODE_SEGMENTS_BYTES + numberOfProcessors * sizeof(TSSDescriptor);
    if (requiredBytesForDescriptorTable > MAX_BYTES_GDT) {
        EXIT_WITH_MESSAGE {
            INFO(STRING("The GDT is too big!\nSize: "));
            INFO(requiredBytesForDescriptorTable);
            INFO(STRING(" while maximum allowed is: "));
            INFO(MAX_BYTES_GDT, NEWLINE);
        }
    }
    KFLUSH_AFTER {
        INFO(STRING("Total bytes required for GDT: "));
        INFO(requiredBytesForDescriptorTable, NEWLINE);
    }

    SegmentDescriptor *GDT = (SegmentDescriptor *)allocateKernelStructure(
        requiredBytesForDescriptorTable, sizeof(TSSDescriptor), false, scratch);
    GDT[0] = (SegmentDescriptor){0}; // null segment;
    GDT[1] = (SegmentDescriptor){
        .limit_15_0 = 0, // ignored in 64-bit
        .base_15_0 = 0,  // ignored in 64-bit
        .base_23_16 = 0, // ignored in 64-bit
        .type = 0xA,     // Execute | Read
        .systemDescriptor = 1,
        .descriptorprivilegelevel = 0,
        .present = 1,
        .limit_19_16 = 0, // ignored in 64-bit
        .availableToUse = 0,
        .use64Bit = 1,
        .defaultOperationSizeUpperBound = 0, // 0 for correctness but ignored
        .granularity = 0,                    // ignored in 64-bit
        .base_31_24 = 0                      // ignored in 64-bit
    }; // kernel code segment
    GDT[2] = (SegmentDescriptor){
        .limit_15_0 = 0, // ignored in 64-bit
        .base_15_0 = 0,  // ignored in 64-bit
        .base_23_16 = 0, // ignored in 64-bit
        .type = 0x2,     // Read | Write
        .systemDescriptor = 1,
        .descriptorprivilegelevel = 0,
        .present = 1,
        .limit_19_16 = 0, // ignored in 64-bit
        .availableToUse = 0,
        .use64Bit = 0,                       // ignored in 64-bit
        .defaultOperationSizeUpperBound = 0, // 0 for correctness but ignored
        .granularity = 0,                    // ignored in 64-bit
        .base_31_24 = 0                      // ignored in 64-bit
    }; // kernel data segment

    // We are skipping GDT[3] here because the TSSDescriptor is 16 bytes, so
    // making sure it's nicely aligned and making it easier to count
    TSSDescriptor *tssDescriptors = (TSSDescriptor *)&GDT[4];

    // Task State Segment is packed, but the CPU sometimes writes to the Task
    // State Segment itself, so ensure that the structures are laid out to avoid
    // false sharing.
    U64 bytesPerTSS =
        ALIGN_UP_VALUE(sizeof(TaskStateSegment), cacheLineSizeBytes);
    KFLUSH_AFTER {
        INFO(STRING("Size in bytes per TSS: "));
        INFO(bytesPerTSS, NEWLINE);
    }

    U8 *TSSes = (U8 *)allocateKernelStructure(bytesPerTSS * numberOfProcessors,
                                              bytesPerTSS, false, scratch);
    PhysicalBasePage *interruptStacks =
        (PhysicalBasePage *)allocateKernelStructure(
            sizeof(PhysicalBasePage) * numberOfProcessors,
            alignof(PhysicalBasePage), false, scratch);

    for (U64 i = 0; i < numberOfProcessors; i++) {
        TaskStateSegment *TSS = (TaskStateSegment *)(TSSes + (bytesPerTSS * i));
        TSS->IOMapBaseAddress =
            sizeof(TaskStateSegment); // but limit is set to max TSS, so the CPU
                                      // understands that there is no io
                                      // permission bitmap.
        // Stack grows down, so we already multiply by 1 to account for that.
        TSS->ist1 = (U64)interruptStacks[i].data + INTERRUPT_STACK_SIZE * 1;
        TSS->ist2 = (U64)interruptStacks[i].data + INTERRUPT_STACK_SIZE * 2;
        TSS->ist3 = (U64)interruptStacks[i].data + INTERRUPT_STACK_SIZE * 3;
        TSS->ist4 = (U64)interruptStacks[i].data + INTERRUPT_STACK_SIZE * 4;

        KFLUSH_AFTER {
            INFO(STRING("TSS ist1 stack: "));
            INFO(TSS->ist1, NEWLINE);
            INFO(STRING("TSS ist2 stack: "));
            INFO(TSS->ist2, NEWLINE);
            INFO(STRING("TSS ist3 stack: "));
            INFO(TSS->ist3, NEWLINE);
            INFO(STRING("TSS ist4 stack: "));
            INFO(TSS->ist4, NEWLINE);
        }

        tssDescriptors[i] =
            (TSSDescriptor){(SegmentDescriptor){
                                .limit_15_0 = sizeof(TaskStateSegment) - 1,
                                .base_15_0 = (U64)(TSS) & 0xFFFF,
                                .base_23_16 = ((U64)(TSS) >> 16) & 0xFF,
                                .type = 0x9, // 0b1001 = 64 bit TSS (available)
                                .systemDescriptor = 0,
                                .descriptorprivilegelevel = 0,
                                .present = 1,
                                .limit_19_16 = 0,
                                .availableToUse = 0,
                                .use64Bit = 0,
                                .defaultOperationSizeUpperBound = 0,
                                .granularity = 0,
                                .base_31_24 = ((U64)(TSS) >> 24) & 0xFF,
                            },
                            .base_63_32 = ((U64)(TSS) >> 32) & 0xFFFFFFFF,
                            .reserved_1 = 0, .zero_4 = 0, .reserved_2 = 0};
    }

    gdtDescriptor = (DescriptorTableRegister){
        .limit = ((U16)requiredBytesForDescriptorTable) - 1, .base = (U64)GDT};
}

void bootstrapProcessorWork(U16 cacheLineSizeBytes, Arena scratch) {
    // NOTE: What the fuck does this do and why?
    disablePIC();

    // TODO: Find out number of processors!
    prepareDescriptors(1, cacheLineSizeBytes, scratch);

    // Maybe when there is other CPUs in here??
    //    // NOTE: WHY????
    //    globals.st->boot_services->stall(100000);

    // NOTE: Whaw the fuck is this and why are you here?
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
    if (leaf15.ebx && leaf15.ecx > 10000) {
        return (leaf15.ecx * (leaf15.ebx / leaf15.eax)) / 1000000;
    } else {
        CPUIDResult leaf16 = CPUID(0x16);
        if (leaf16.eax > 1000) {
            return leaf16.eax;
        }
    }

    U64 currentCycles = currentCycleCounter(false, false);
    globals.st->boot_services->stall(CALIBRATION_MICROSECONDS);
    U64 endCycles = currentCycleCounter(false, false);
    return (endCycles - currentCycles) / CALIBRATION_MICROSECONDS;
}

ArchParamsRequirements getArchParamsRequirements() {
    return (ArchParamsRequirements){.bytes = sizeof(X86ArchParams),
                                    .align = alignof(X86ArchParams)};
}

void initRootVirtualMemoryInKernel() {
    rootPageTable = getZeroedPageTable();

    KFLUSH_AFTER {
        INFO(STRING("root page table memory location: "));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)rootPageTable, NEWLINE);
    }
}

// NOTE: Should be enough until put into final kernel position.
static constexpr auto INITIAL_VIRTUAL_MEMORY_REGIONS = 16;
static constexpr auto INITIAL_VIRTUAL_MAPPING_SIZES = 128;

void initKernelMemoryManagement(U64 startingAddress, U64 endingAddress,
                                Arena scratch) {
    physicalMA = (RedBlackMMTreeWithFreeList){0};

    virtualMA.tree = nullptr;
    createDynamicArray(
        INITIAL_VIRTUAL_MEMORY_REGIONS, sizeof(*virtualMA.nodes.buf),
        alignof(*virtualMA.nodes.buf), (void_max_a *)&virtualMA.nodes, scratch);
    createDynamicArray(INITIAL_VIRTUAL_MEMORY_REGIONS,
                       sizeof(*virtualMA.freeList.buf),
                       alignof(*virtualMA.freeList.buf),
                       (void_max_a *)&virtualMA.freeList, scratch);

    // Initial size > 2 so no bounds checking here.
    MMNode *node = &virtualMA.nodes.buf[virtualMA.nodes.len];
    virtualMA.nodes.len++;
    node->memory = (Memory){.start = startingAddress,
                            .bytes = LOWER_HALF_END - startingAddress};
    (void)insertMMNode(&virtualMA.tree, node);

    node = &virtualMA.nodes.buf[virtualMA.nodes.len];
    virtualMA.nodes.len++;
    node->memory = (Memory){.start = HIGHER_HALF_START,
                            .bytes = endingAddress - HIGHER_HALF_START};
    (void)insertMMNode(&virtualMA.tree, node);

    virtualMemorySizeMapper.tree = nullptr;
    createDynamicArray(INITIAL_VIRTUAL_MAPPING_SIZES,
                       sizeof(*virtualMemorySizeMapper.nodes.buf),
                       alignof(*virtualMemorySizeMapper.nodes.buf),
                       (void_max_a *)&virtualMemorySizeMapper.nodes, scratch);
    createDynamicArray(INITIAL_VIRTUAL_MAPPING_SIZES,
                       sizeof(*virtualMemorySizeMapper.freeList.buf),
                       alignof(*virtualMemorySizeMapper.freeList.buf),
                       (void_max_a *)&virtualMemorySizeMapper.freeList,
                       scratch);
}

void fillArchParams(void *archParams, Arena scratch) {
    X86ArchParams *x86ArchParams = (X86ArchParams *)archParams;

    U32 maxBasicCPUID = CPUID(0x0).eax;
    if (maxBasicCPUID < BASIC_MAX_REQUIRED_PARAMETER) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support required CPUID of "));
            ERROR(BASIC_MAX_REQUIRED_PARAMETER, NEWLINE);
        }
    }

    CPUIDResult processorInfoAndFeatureBits =
        CPUID(BASIC_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER);
    BASICCPUFeatures features = {.ecx = processorInfoAndFeatureBits.ecx,
                                 .edx = processorInfoAndFeatureBits.edx};
    if (!features.APIC) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support APIC")); }
    }

    U16 cacheLineSizeBytes = (processorInfoAndFeatureBits.ebx >> 8 & 0xFF) * 8;
    KFLUSH_AFTER {
        INFO(STRING("Cache line size is: \n"));
        INFO(cacheLineSizeBytes, NEWLINE);
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
        KFLUSH_AFTER {
            INFO(STRING("Support for AVX512 found\nCan set CR4.LA57 = 1 to "
                        "turn it on."));
        }
    } else {
        KFLUSH_AFTER { INFO(STRING("No Support for AVX512 found\n")); }
    }

    KFLUSH_AFTER {
        INFO(STRING("Configuring XSAVE to enable FPU / SSE / AVX\n"));
    }
    enableAndConfigureXSAVE(supportsAVX512);

    if (extendedProcessorFeatures.ecx & (1 << 16)) {
        KFLUSH_AFTER { INFO(STRING("Support for 5 level-paging found\n")); }
    } else {
        KFLUSH_AFTER { INFO(STRING("No Support for 5 level-paging found\n")); }
    }

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
    bootstrapProcessorWork(cacheLineSizeBytes, scratch);

    KFLUSH_AFTER { INFO(STRING("Calibrating timer\n")); }
    x86ArchParams->tscFrequencyPerMicroSecond = calibrateWait();

    x86ArchParams->XSAVELocation = XSAVEAddress;

    x86ArchParams->rootPageMetaData.children =
        (struct PackedPageMetaDataNode *)rootPageMetaData.children;
    x86ArchParams->rootPageMetaData.metaData.entriesMapped =
        rootPageMetaData.metaData.entriesMapped;
    x86ArchParams->rootPageMetaData.metaData
        .entriesMappedWithSmallerGranularity =
        rootPageMetaData.metaData.entriesMappedWithSmallerGranularity;
}
