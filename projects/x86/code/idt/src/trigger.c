#include "abstraction/interrupts.h"
#include "x86/fault.h"
#include "x86/idt.h"

void interruptUnexpectedError() { triggerFault(FAULT_UNEXPECTED_FAILURE); }

void interruptNoMorePhysicalMemory() {
    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}
void interruptNoMoreVirtualMemory() {
    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}
