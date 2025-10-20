#ifndef ABSTRACTION_TIME_H
#define ABSTRACTION_TIME_H

#include "shared/macros.h"
#include "shared/types/numeric.h"

[[nodiscard]] U64 cycleCounterGet(bool previousFinished,
                                      bool blocksSubsequent);
void waitBlock(U64 microSeconds);

#define BENCHMARK(cycleCounter)                                                \
    for (U64 MACRO_VAR(i) = 0,                                                 \
             MACRO_VAR(startCycles) = cycleCounterGet(false, false);       \
         MACRO_VAR(i) < 1;                                                     \
         MACRO_VAR(i) = 1, cycleCounter = cycleCounterGet(false, false) -  \
                                          MACRO_VAR(startCycles))

#endif
