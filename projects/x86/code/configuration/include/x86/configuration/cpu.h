#ifndef X86_CONFIGURATION_CPU_H
#define X86_CONFIGURATION_CPU_H

#include "shared/types/numeric.h"

U64 rdmsr(U32 msr);
void wrmsr(U32 msr, U64 value);

void flushCPUCaches();

typedef U32 CPUIDLeaf;

// NOTE: constexpr c26 for max calculation
//
// Basic CPUID leafs
static constexpr CPUIDLeaf BASIC_MAX_VALUE_AND_MANUFACTURER = 0x0;
static constexpr CPUIDLeaf BASIC_PROCESSOR_INFO_AND_FEATURE_BITS = 0x1;
static constexpr CPUIDLeaf EXTENDED_FEATURES = 0x7;
static constexpr CPUIDLeaf XSAVE_CPU_SUPPORT = 0xD;
static constexpr CPUIDLeaf TSC_AND_CORE_CRYSTAL_FREQ = 0x15;
static constexpr CPUIDLeaf PROCESSOR_AND_BUS_FREQ = 0x16;

// NOTE: constexpr c26 for max calculation
//
// Extended CPUID leafs
static constexpr CPUIDLeaf EXTENDED_MAX_VALUE = 0x80000000;
static constexpr CPUIDLeaf EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS_PARAMETER =
    0x80000001;
static constexpr CPUIDLeaf EXTENDED_PROCESSOR_POWER_MANEGEMENT_OPERATION =
    0x80000007;
static constexpr CPUIDLeaf EXTENDED_MAX_REQUIRED =
    EXTENDED_PROCESSOR_POWER_MANEGEMENT_OPERATION;

typedef struct {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
} CPUIDResult;
CPUIDResult CPUID(U32 leaf);

CPUIDResult CPUIDWithSubleaf(U32 leaf, U32 subleaf);
void disablePIC();
U64 CR3();
U64 CR2();

#endif
