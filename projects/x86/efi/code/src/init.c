#include "x86/efi/init.h"
#include "abstraction/efi.h"
#include "x86/efi/avx512.h"
#include "x86/efi/time.h"

#include "abstraction/interrupts.h"
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
#include "efi/memory/virtual.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"
#include "shared/memory/management/status.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"
#include "x86/configuration/features.h"
#include "x86/efi-to-kernel/params.h"
#include "x86/efi/gdt.h"
#include "x86/fault.h"
#include "x86/gdt.h"
#include "x86/memory/definitions.h"
#include "x86/memory/virtual.h"

static constexpr auto MANUFACTURER_STRING_LEN = 12;

typedef struct {
    Manufacturer manufacturer;
    U8 *string;
} Vendor;

void manufacturerCheck(Manufacturer actual, Manufacturer expected) {
    if (actual != expected) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("incompatible manufacturer! Detected "));
            ERROR(expected);
            ERROR(STRING(" while build is for "));
            ERROR(actual, .flags = NEWLINE);
        }
    }
}

void CPUIDCheck(U32 actualMax, U32 expectedMax) {
    if (actualMax < expectedMax) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support required CPUID of "));
            ERROR(expectedMax, .flags = NEWLINE);
        }
    }
}

static Vendor VENDORS[] = {
    {.manufacturer = INTEL, .string = (U8 *)"GenuineIntel"},
    {.manufacturer = AMD, .string = (U8 *)"AuthenticAMD"}};

static Manufacturer manufacturerGet(U8 manufacturer[MANUFACTURER_STRING_LEN]) {
    KFLUSH_AFTER {
        INFO(STRING("manufacturer: "));
        INFO(STRING_LEN(manufacturer, MANUFACTURER_STRING_LEN),
             .flags = NEWLINE);
    }

    for (U32 i = 0; i < COUNTOF(VENDORS); i++) {
        if (!memcmp(VENDORS[i].string, manufacturer, MANUFACTURER_STRING_LEN)) {
            return VENDORS[i].manufacturer;
        }
    }

    EXIT_WITH_MESSAGE { ERROR(STRING("Unknown manufacturer!!!\n")); }
    __builtin_unreachable();
}

static constexpr auto MAX_BYTES_GDT = 64 * KiB;
// the GDT can contain up to 8192 descriptors 8-byte = 655365 bytes = 64 KiB
// - null              = 8 bytes
// - kernel code       = 8 bytes
// - kernel data       = 8 butes
// - padding           = 8 butes
// - x tss descriptors = 16 * x bytes !!! should be aligned to 16 bytes for
// performance reasons.
// So, we align the GDT to 16 bytes, so everything is at least self-aligned.
static void prepareDescriptors(U16 numberOfProcessors, U16 cacheLineSizeBytes,
                               U64 memoryVirtualAddressAvailable) {
    U32 requiredBytesForDescriptorTable =
        CODE_SEGMENTS_BYTES + numberOfProcessors * sizeof(TSSDescriptor);
    if (requiredBytesForDescriptorTable > MAX_BYTES_GDT) {
        EXIT_WITH_MESSAGE {
            INFO(STRING("The GDT is too big!\nSize: "));
            INFO(requiredBytesForDescriptorTable);
            INFO(STRING(" while maximum allowed is: "));
            INFO(MAX_BYTES_GDT, .flags = NEWLINE);
        }
    }
    KFLUSH_AFTER {
        INFO(STRING("Total bytes required for GDT: "));
        INFO(requiredBytesForDescriptorTable, .flags = NEWLINE);
    }

    SegmentDescriptor *GDT = (SegmentDescriptor *)NEW(
        &globals.kernelPermanent, U8, .count = requiredBytesForDescriptorTable,
        .align = sizeof(TSSDescriptor));
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
    U32 bytesPerTSS =
        (U32)alignUp(sizeof(TaskStateSegment), cacheLineSizeBytes);
    KFLUSH_AFTER {
        INFO(STRING("Size in bytes per TSS: "));
        INFO(bytesPerTSS, .flags = NEWLINE);
    }

    U8 *TSSes =
        NEW(&globals.kernelPermanent, U8,
            .count = bytesPerTSS * numberOfProcessors, .align = bytesPerTSS);

    for (typeof(numberOfProcessors) i = 0; i < numberOfProcessors; i++) {
        TaskStateSegment *perCPUTSS =
            (TaskStateSegment *)(TSSes + (bytesPerTSS * i));
        perCPUTSS->IOMapBaseAddress =
            sizeof(*perCPUTSS); // but limit is set to max TSS, so the CPU
                                // understands that there is no io
                                // permission bitmap.

        KFLUSH_AFTER {
            for (typeof_unqual(INTERRUPT_STACK_TABLE_COUNT) j = 0;
                 j < INTERRUPT_STACK_TABLE_COUNT; j++) {
                StackResult stackResult = stackCreateAndMap(
                    memoryVirtualAddressAvailable, IST_STACK_SIZE, false);
                memoryVirtualAddressAvailable =
                    stackResult.virtualMemoryFirstAvailable;
                perCPUTSS->ists[j] = stackResult.stackVirtualTop;
            }
        }

        tssDescriptors[i] =
            (TSSDescriptor){(SegmentDescriptor){
                                .limit_15_0 = sizeof(*perCPUTSS) - 1,
                                .base_15_0 = (U64)(perCPUTSS) & 0xFFFF,
                                .base_23_16 = ((U64)(perCPUTSS) >> 16) & 0xFF,
                                .type = 0x9, // 0b1001 = 64 bit TSS (available)
                                .systemDescriptor = 0,
                                .descriptorprivilegelevel = 0,
                                .present = 1,
                                .limit_19_16 = 0,
                                .availableToUse = 0,
                                .use64Bit = 0,
                                .defaultOperationSizeUpperBound = 0,
                                .granularity = 0,
                                .base_31_24 = ((U64)(perCPUTSS) >> 24) & 0xFF,
                            },
                            .base_63_32 = ((U64)(perCPUTSS) >> 32) & 0xFFFFFFFF,
                            .reserved_1 = 0, .zero_4 = 0, .reserved_2 = 0};
    }

    gdtDescriptor = (DescriptorTableRegister){
        .limit = ((U16)requiredBytesForDescriptorTable) - 1, .base = (U64)GDT};
}

static void bootstrapProcessorWork(U16 cacheLineSizeBytes,
                                   U64 memoryVirtualAddressAvailable) {
    // NOTE: What the fuck does this do and why?
    disablePIC();

    // TODO: Find out number of processors!
    prepareDescriptors(1, cacheLineSizeBytes, memoryVirtualAddressAvailable);

    // Maybe when there is other CPUs in here??
    //    // NOTE: WHY????
    //    globals.st->boot_services->stall(100000);

    // NOTE: Whaw the fuck is this and why are you here?
    asm volatile("pause" : : : "memory"); // memory barrier
}

void initRootVirtualMemoryInKernel() {
    rootPageTable = getZeroedPageTable();

    KFLUSH_AFTER {
        INFO(STRING("root page table memory: "));
        memoryAppend((Memory){
            .start = (U64)rootPageTable,
            .bytes = virtualStructBytes[VIRTUAL_PAGE_TABLE_ALLOCATION]});
        INFO(STRING("\n"));
    }
}

// NOTE: Should be enough until put into final kernel position.
static constexpr auto INITIAL_VIRTUAL_MAPPING_SIZES = 128;

void initKernelMemoryManagement(U64 startingAddress, U64 endingAddress) {
    Exponent orderCount =
        buddyOrderCountOnLargestPageSize(BUDDY_VIRTUAL_PAGE_SIZE_MAX);
    U64 *backingBuffer =
        NEW(&globals.kernelPermanent, U64,
            .count = orderCount * BUDDY_BLOCKS_CAPACITY_PER_ORDER_DEFAULT);
    buddyInit(&buddyVirtual, backingBuffer,
              BUDDY_BLOCKS_CAPACITY_PER_ORDER_DEFAULT, orderCount);
    if (setjmp(buddyVirtual.memoryExhausted)) {
        interruptNoMoreVirtualMemory();
    }
    if (setjmp(buddyVirtual.backingBufferExhausted)) {
        interruptNoMoreBuffer();
    }

    Memory freeMemory = {.start = startingAddress,
                         .bytes = LOWER_HALF_END - startingAddress};
    buddyFree(&buddyVirtual, freeMemory);

    freeMemory = (Memory){.start = HIGHER_HALF_START,
                          .bytes = endingAddress - HIGHER_HALF_START};
    buddyFree(&buddyVirtual, freeMemory);

    memoryMapperSizes.tree = nullptr;
    nodeAllocatorInit(
        &memoryMapperSizes.nodeAllocator,
        (void_a){.buf = NEW(&globals.kernelTemporary,
                            typeof(*memoryMapperSizes.tree),
                            .count = INITIAL_VIRTUAL_MAPPING_SIZES),
                 .len = INITIAL_VIRTUAL_MAPPING_SIZES *
                        sizeof(*memoryMapperSizes.tree)},
        (void_a){.buf = NEW(&globals.kernelTemporary, typeof(void *),
                            .count = INITIAL_VIRTUAL_MAPPING_SIZES),
                 .len = INITIAL_VIRTUAL_MAPPING_SIZES * sizeof(void *)},
        sizeof(*memoryMapperSizes.tree), alignof(*memoryMapperSizes.tree));
}

static constexpr auto XSAVE_ALIGNMENT = 64;
void fillArchParams(void *archParams, U64 memoryVirtualAddressAvailable) {
    X86ArchParams *x86ArchParams = (X86ArchParams *)archParams;

    U32 manufacturerString[3];
    CPUIDResult CPUIDMaxAndManufacturer =
        CPUID(BASIC_MAX_VALUE_AND_MANUFACTURER);
    manufacturerString[0] = CPUIDMaxAndManufacturer.ebx;
    manufacturerString[1] = CPUIDMaxAndManufacturer.edx;
    manufacturerString[2] = CPUIDMaxAndManufacturer.ecx;

    Manufacturer manufacturer = manufacturerGet((U8 *)manufacturerString);

    CPUIDResult processorInfoAndFeatureBits =
        CPUID(BASIC_PROCESSOR_INFO_AND_FEATURE_BITS);

    processorVersionCheck(manufacturer, &CPUIDMaxAndManufacturer,
                          &processorInfoAndFeatureBits);

    CPUIDCheck(CPUID(EXTENDED_MAX_VALUE).eax, EXTENDED_MAX_REQUIRED);

    BASICCPUFeatures features = {.ecx = processorInfoAndFeatureBits.ecx,
                                 .edx = processorInfoAndFeatureBits.edx};
    if (!features.APIC) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support APIC\n")); }
    }

    U16 cacheLineSizeBytes = (processorInfoAndFeatureBits.ebx >> 8 & 0xFF) * 8;
    KFLUSH_AFTER {
        INFO(STRING("Cache line size: "));
        INFO(cacheLineSizeBytes, .flags = NEWLINE);
    }

    // U32 BSPID = processorInfoAndFeatureBits.ebx >> 24;

    if (!features.TSC) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support Time Stamp Counter\n"));
        }
    }

    if (!features.TSC) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support model-specific registers and "
                         "rdmsr/wrmsr!\n"));
        }
    }

    if (!features.PGE) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support global memory paging!\n"));
        }
    }
    KFLUSH_AFTER { INFO(STRING("Enabling PGE\n")); }
    CPUEnablePGE();

    if (!features.FPU) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support FPU!\n")); }
    }
    KFLUSH_AFTER { INFO(STRING("Enabling FPU\n")); }
    CPUEnableFPU();

    if (!features.PAT) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support PAT!\n")); }
    }
    KFLUSH_AFTER { INFO(STRING("Configuring PAT\n")); }
    CPUConfigurePAT();

    if (!features.SSE) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support SSE!\n")); }
    }
    CPUEnableSSE();

    if (!features.XSAVE) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support XSAVE!\n")); }
    }
    if (!features.AVX) {
        EXIT_WITH_MESSAGE { ERROR(STRING("CPU does not support AVX256!\n")); }
    }

    CPUIDResult extendedProcessorFeatures = CPUID(EXTENDED_FEATURES);
    bool AVX512SupportByCPU = extendedProcessorFeatures.ebx & (1 << 16);
    if (AVX512SupportByCPU) {
        INFO(STRING("Support for AVX512 found\n"));
    } else {
        STRING("No Support for AVX512 found\n");
    }
    bool AVX512Support = kernelAVX512Support(AVX512SupportByCPU);

    KFLUSH_AFTER {
        INFO(STRING("Configuring XSAVE to enable FPU / SSE / AVX\n"));
    }
    enableAndConfigureXSAVE(AVX512Support);

    if (extendedProcessorFeatures.ecx & (1 << 16)) {
        KFLUSH_AFTER { INFO(STRING("Support for 5 level-paging found\n")); }
    } else {
        KFLUSH_AFTER { INFO(STRING("No Support for 5 level-paging found\n")); }
    }

    CPUIDResult XSAVECPUSupport = CPUIDWithSubleaf(XSAVE_CPU_SUPPORT, 1);
    if (!(XSAVECPUSupport.eax & (1 << 1))) {
        EXIT_WITH_MESSAGE { ERROR(STRING("No Support for XSAVEC found!\n")); }
    }
    KFLUSH_AFTER { INFO(STRING("Support for XSAVEC found!\n")); }
    U32 XSAVESize = CPUIDWithSubleaf(XSAVE_CPU_SUPPORT, 0).ebx;
    U8 *XSAVEAddress = NEW(&globals.kernelPermanent, U8, .count = XSAVESize,
                           .align = XSAVE_ALIGNMENT, .flags = ZERO_MEMORY);
    KFLUSH_AFTER {
        INFO(STRING("XSAVE space location: "));
        memoryAppend((Memory){.start = (U64)XSAVEAddress, .bytes = XSAVESize});
        INFO(STRING("\n"));
    }
    x86ArchParams->XSAVELocation = XSAVEAddress;

    CPUIDResult extendedProcessorInfoAndFeatureBits =
        CPUID(EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER);
    if (!(extendedProcessorInfoAndFeatureBits.edx & (1 << 26))) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support Huge Pages!\n"));
        }
    }

    KFLUSH_AFTER { INFO(STRING("Bootstrap processor work...\n")); }
    bootstrapProcessorWork(cacheLineSizeBytes, memoryVirtualAddressAvailable);

    U32 processorPowerManagement =
        CPUID(EXTENDED_PROCESSOR_POWER_MANEGEMENT_OPERATION).edx;
    if (!(processorPowerManagement & (1 << 8))) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("CPU does not support invariant TSC!\n"));
        }
    }

    x86ArchParams->tscFrequencyPerMicroSecond =
        timestampFrequencyGet() / MICROSECONDS_PER_SECOND;
    KFLUSH_AFTER {
        INFO(STRING("tsc frequency per microsecond: "));
        INFO(x86ArchParams->tscFrequencyPerMicroSecond, .flags = NEWLINE);
    }

    x86ArchParams->rootPageMetaData.children =
        (struct PackedPageMetaDataNode *)rootPageMetaData.children;
    x86ArchParams->rootPageMetaData.metaData.entriesMapped =
        rootPageMetaData.metaData.entriesMapped;
    x86ArchParams->rootPageMetaData.metaData
        .entriesMappedWithSmallerGranularity =
        rootPageMetaData.metaData.entriesMappedWithSmallerGranularity;
}
