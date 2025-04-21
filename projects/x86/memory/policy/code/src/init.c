#include "abstraction/memory/management/init.h"

#include "x86/memory/physical.h"
#include "x86/memory/policy/virtual.h"

void initMemoryManager(KernelMemory memory) {
    initPhysicalMemoryManager(memory.physical);
    initVirtualMemoryManager(memory.virt);
}
