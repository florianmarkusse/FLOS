#ifndef ABSTRACTION_INTERRUPTS_H
#define ABSTRACTION_INTERRUPTS_H

#include "shared/types/numeric.h"

void initIDT();

__attribute__((noreturn)) void interruptNoMorePhysicalMemory();
__attribute__((noreturn)) void interruptNoMoreVirtualMemory();
__attribute__((noreturn)) void interruptUnexpectedError();

// TODO: remove?
U64 getPageFaults();

extern U64 pageSizeToMap;

#endif
