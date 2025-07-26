#include "x86/gdt.h"
#include "shared/types/numeric.h" // for U64, U32, U16
#include "x86/memory/definitions.h"

typedef struct __attribute__((packed)) {
    union {
        struct {
            U64 limit_15_0 : 16;
            U64 base_15_0 : 16;
            U64 base_23_16 : 8;
            U64 type : 4;
            U64 systemDescriptor : 1;
            U64 descriptorprivilegelevel : 2;
            U64 present : 1;
            U64 limit_19_16 : 4;
            U64 availableToUse : 1; // Ignored, free to use if you dare
            U64 use64Bit : 1;       // 64 bit code if set
            U64 defaultOperationSizeUpperBound : 1;
            U64 granularity : 1;
            U64 base_31_24 : 8; // Ignored in 64-bit
        };
        U64 value;
    };
} SegmentDescriptor;

typedef struct __attribute__((packed)) {
    SegmentDescriptor segment;
    U64 base_63_32 : 32;
    U64 reserved_1 : 8;
    U64 zero_4 : 5;
    U64 reserved_2 : 19;
} TSSDescriptor;

// 104 bytes
typedef struct __attribute__((packed)) {
    U32 reserved_0;
    U64 rsp0;
    U64 rsp1;
    U64 rsp2;
    U32 reserved_1;
    U32 reserved_2;
    U64 ist1;
    U64 ist2;
    U64 ist3;
    U64 ist4;
    U64 ist5;
    U64 ist6;
    U64 ist7;
    U32 reserved_3;
    U32 reserved_4;
    U16 reserved_5;
    U16 IOMapBaseAddress;
} TaskStateSegment;

// typedef struct __attribute__((packed)) {
//     SegmentDescriptor null;
//     SegmentDescriptor kernelCode;
//     SegmentDescriptor kernelData;
//     TSSDescriptor tssDescriptor;
// } GDTTable;

DescriptorTableRegister *prepNewGDT(PhysicalBasePage zeroPages[7]) {
    SegmentDescriptor *GDT = (SegmentDescriptor *)zeroPages[5].data;
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
    TaskStateSegment *TSS = (TaskStateSegment *)zeroPages[0].data;
    TSS->IOMapBaseAddress =
        sizeof(TaskStateSegment); // but limit is set to max TSS, so the CPU
                                  // understands that there is no io permission
                                  // bitmap.
    TSS->ist1 = (U64)zeroPages[1].data;
    TSS->ist2 = (U64)zeroPages[2].data;
    TSS->ist3 = (U64)zeroPages[3].data;
    TSS->ist4 = (U64)zeroPages[4].data;
    tssDescriptors[0] =
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

    DescriptorTableRegister *GDTRegister =
        (DescriptorTableRegister *)zeroPages[6].data;
    // the GDT can contain up to 8192 8-byte descriptors
    // - null              = 8 bytes
    // - kernel code       = 8 bytes
    // - kernel data       = 8 butes
    // - x tss descriptors = 16 * x bytes !!! should be aligned to 16 bytes for
    // performance reasons. calling code should ensure that!
    // TODO: fix the math below here!
    *GDTRegister = (DescriptorTableRegister){.limit = 48 - 1, .base = (U64)GDT};
    return GDTRegister;
}
void enableNewGDT(DescriptorTableRegister *GDTRegister) {
    asm volatile("lgdt %0;" // Load new Global Descriptor Table

                 "movq $0x08, %%rax;"
                 "pushq %%rax;"           // Push kernel code segment
                 "leaq 1f(%%rip), %%rax;" // Use relative offset for label
                 "pushq %%rax;"           // Push return address
                 "lretq;" // Far return, pop return address into IP,
                          //   and pop code segment into CS
                 "1:"
                 "movw $0x10, %%ax;" // 0x10 = kernel data segment
                 "movw %%ax, %%ds;"  // Load data segment registers
                 "movw %%ax, %%es;"
                 "movw %%ax, %%fs;"
                 "movw %%ax, %%gs;"
                 "movw %%ax, %%ss;"

                 :
                 : "m"(*GDTRegister)
                 : "rax", "memory");

    asm volatile(
        // TODO: make function for this ltr stuff!!!!
        "movw $0x20, %%ax;" // 0x20 = tss segment offset in gdt
        "ltr %%ax;"         // Load task register, with tss offset
        :
        :
        : "rax", "memory");
}
