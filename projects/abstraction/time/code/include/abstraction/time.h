#ifndef ABSTRACTION_TIME_H
#define ABSTRACTION_TIME_H

#include "shared/macros.h"
#include "shared/types/numeric.h"

U64 getCyclesPerMicroSecond();
U64 currentCycleCounter(bool previousFinished, bool blocksSubsequent);
void blockingWait(U64 microSeconds);

#define BENCHMARK(cycleCounter)                                                \
    for (U64 MACRO_VAR(i) = 0,                                                 \
             MACRO_VAR(startCycles) = currentCycleCounter(false, false);       \
         MACRO_VAR(i) < 1;                                                     \
         MACRO_VAR(i) = 1, cycleCounter = currentCycleCounter(false, false) -  \
                                          MACRO_VAR(startCycles))

#endif
