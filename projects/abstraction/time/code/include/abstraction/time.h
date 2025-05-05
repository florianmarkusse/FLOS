#ifndef ABSTRACTION_TIME_H
#define ABSTRACTION_TIME_H

#include "shared/types/numeric.h"

U64 currentCycleCounter();
void blockingWait(U64 microSeconds);

#endif
