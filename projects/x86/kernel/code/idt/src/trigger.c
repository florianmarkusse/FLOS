#include "abstraction/interrupts.h"
#include "x86/kernel/idt.h"

void interruptPhysicalMemory() {
    faultTrigger(FAULT_NO_MORE_PHYSICAL_MEMORY);
}

void interruptVirtualMemory() {
    faultTrigger(FAULT_NO_MORE_VIRTUAL_MEMORY);
}

void interruptVirtualMemoryMapper() {
    faultTrigger(FAULT_NO_MORE_VIRTUAL_MEMORY_MAPPER);
}

void interruptBuffer() { faultTrigger(FAULT_NO_MORE_BUFFER); }

void interruptUnexpectedError() { faultTrigger(FAULT_UNEXPECTED_FAILURE); }
