#include "abstraction/time.h"
#include "shared/types/numeric.h"

U64 cycleCounterGet(bool previousFinished, bool blocksSubsequent) {
    U32 edx;
    U32 eax;
    U32 ecx;
    if (previousFinished) {
        asm volatile("mfence");
    }
    asm volatile("rdtscp"
                 : "=a"(eax), "=d"(edx), "=c"(ecx)); // ecx is the processor ID
    if (blocksSubsequent) {
        asm volatile("lfence");
    }
    return ((U64)edx << 32) | eax;
}

// NOTE: 1 GigaHertz = 1000 cycles per microsecond default - really should
// calibrate this!
U64 tscCyclesPerMicroSecond = 1000;

// 1 millionth of a second
void waitBlock(U64 microSeconds) {
    U64 endInCycles = cycleCounterGet(false, false) +
                      microSeconds * tscCyclesPerMicroSecond;
    while (cycleCounterGet(false, false) < endInCycles) {
        asm volatile("" ::: "memory");
    }
}
