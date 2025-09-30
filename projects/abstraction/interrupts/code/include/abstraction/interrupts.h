#ifndef ABSTRACTION_INTERRUPTS_H
#define ABSTRACTION_INTERRUPTS_H

#include "shared/types/numeric.h"

// NOTE: this is 3 projects in one actually

// arch specific way to set up interrupts, one-time thing and is different per
// project perhaps, i.e., each OS can have different setups - should only be
// called in freestanding
void setupInterrupts();

// arch specific - should only be called in freestanding
void enableInterrupts();
void disableInterrupts();

// arch specific and / or emulated by env such as efi
__attribute__((noreturn)) void interruptNoMorePhysicalMemory();
__attribute__((noreturn)) void interruptNoMoreVirtualMemory();
__attribute__((noreturn)) void interruptNoMoreVirtualMemoryMapper();

__attribute__((noreturn)) void interruptNoMoreBuffer();

__attribute__((noreturn)) void interruptUnexpectedError();

extern U64 currentNumberOfPageFaults;

#endif
