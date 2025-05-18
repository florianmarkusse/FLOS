#ifndef X86_CONFIGURATION_CPU_H
#define X86_CONFIGURATION_CPU_H

#include "shared/types/numeric.h"

U64 rdmsr(U32 msr);
void wrmsr(U32 msr, U64 value);

void flushCPUCaches();

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
