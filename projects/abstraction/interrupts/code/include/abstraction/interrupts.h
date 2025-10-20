#ifndef ABSTRACTION_INTERRUPTS_H
#define ABSTRACTION_INTERRUPTS_H

#include "shared/types/numeric.h"

// NOTE: this is 3 projects in one actually

// arch specific way to set up interrupts, one-time thing and is different per
// project perhaps, i.e., each OS can have different setups - should only be
// called in freestanding
void interruptsInit();

// arch specific - should only be called in freestanding
void interruptsEnable();
void interruptsDisable();

// arch specific and / or emulated by env such as efi
__attribute__((noreturn)) void interruptPhysicalMemory();
__attribute__((noreturn)) void interruptVirtualMemory();
__attribute__((noreturn)) void interruptVirtualMemoryMapper();

__attribute__((noreturn)) void interruptBuffer();

__attribute__((noreturn)) void interruptUnexpectedError();

extern U64 pageFaultsCurrent;

typedef struct Registers Registers;
void faultHandler(Registers *regs);

#endif
