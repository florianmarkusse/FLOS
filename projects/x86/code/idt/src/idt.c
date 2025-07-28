#include "abstraction/interrupts.h"

void enableInterrupts() { asm volatile("sti;"); }
