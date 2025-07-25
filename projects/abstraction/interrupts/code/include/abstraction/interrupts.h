#ifndef ABSTRACTION_INTERRUPTS_H
#define ABSTRACTION_INTERRUPTS_H

#include "shared/types/numeric.h"

void setupInterrupts();
void enableInterrupts();

__attribute__((noreturn)) void interruptNoMorePhysicalMemory();
__attribute__((noreturn)) void interruptNoMoreVirtualMemory();
__attribute__((noreturn)) void interruptUnexpectedError();

extern U64 currentNumberOfPageFaults;

#endif
