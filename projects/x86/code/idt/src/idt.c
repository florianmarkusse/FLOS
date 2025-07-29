#include "abstraction/interrupts.h"

void enableInterrupts() { asm volatile("sti;"); }

void disableInterrupts() { asm volatile("cli;"); }
