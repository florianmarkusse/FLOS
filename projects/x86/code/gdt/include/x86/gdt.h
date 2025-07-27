#ifndef X86_GDT_H
#define X86_GDT_H

#include "shared/types/numeric.h"
#include "x86/memory/definitions.h"

typedef struct __attribute__((packed)) {
    U16 limit;
    U64 base;
} DescriptorTableRegister;

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

static constexpr auto CODE_SEGMENTS_BYTES = 4 * sizeof(SegmentDescriptor);

void loadGDTAndSegments(DescriptorTableRegister *GDT);
void loadTaskRegister(U16 processorID);

#endif
