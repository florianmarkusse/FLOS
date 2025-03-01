#ifndef X86_CONFIGURATION_CPU_H
#define X86_CONFIGURATION_CPU_H

#include "shared/types/types.h"

U64 rdmsr(U32 msr);
void wrmsr(U32 msr, U64 value);

void flushTLB();
void flushCPUCaches();

extern U64 cyclesPerMicroSecond;
void wait(U64 microSeconds);

typedef struct {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
} CPUIDResult;
CPUIDResult CPUID(U32 functionID);
void disablePIC();
U64 CR3();

#endif
