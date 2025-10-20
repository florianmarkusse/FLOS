#include "abstraction/interrupts.h"

void interruptsEnable() { asm volatile("sti;"); }

void interruptsDisable() { asm volatile("cli;"); }
