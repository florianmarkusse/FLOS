#include "abstraction/interrupts.h"
#include "x86/kernel/idt.h"

void interruptPhysicalMemory() {
    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}

void interruptVirtualMemory() {
    triggerFault(FAULT_NO_MORE_VIRTUAL_MEMORY);
}

void interruptVirtualMemoryMapper() {
    triggerFault(FAULT_NO_MORE_VIRTUAL_MEMORY_MAPPER);
}

void interruptBuffer() { triggerFault(FAULT_NO_MORE_BUFFER); }

void interruptUnexpectedError() { triggerFault(FAULT_UNEXPECTED_FAILURE); }
