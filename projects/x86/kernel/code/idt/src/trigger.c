#include "abstraction/interrupts.h"
#include "x86/kernel/idt.h"

void interruptNoMorePhysicalMemory() {
    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}

void interruptNoMoreVirtualMemory() {
    triggerFault(FAULT_NO_MORE_VIRTUAL_MEMORY);
}

void interruptNoMoreVirtualMemoryMapper() {
    triggerFault(FAULT_NO_MORE_VIRTUAL_MEMORY_MAPPER);
}

void interruptNoMoreBuffer() { triggerFault(FAULT_NO_MORE_BUFFER); }

void interruptUnexpectedError() { triggerFault(FAULT_UNEXPECTED_FAILURE); }
