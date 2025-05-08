#include "abstraction/time.h"
#include "shared/types/numeric.h"

U64 currentCycleCounter() {
    U32 edx;
    U32 eax;
    U32 ecx;
    asm volatile("rdtscp"
                 : "=a"(eax), "=d"(edx), "=c"(ecx)); // ecx is the processor ID
    return ((U64)edx << 32) | eax;
}

// NOTE: 1 GigaHertz = 1000 cycles per microsecond default - really should
// calibrate this!
U64 cyclesPerMicroSecond = 1000;

U64 getCyclesPerMicroSecond() { return cyclesPerMicroSecond; }

// 1 millionth of a second
void blockingWait(U64 microSeconds) {
    U64 endInCycles =
        currentCycleCounter() + microSeconds * cyclesPerMicroSecond;
    while (currentCycleCounter() < endInCycles) {
        asm volatile("" ::: "memory");
    }
}
