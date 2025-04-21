#include "freestanding/memory/policy.h"
#include "freestanding/memory/physical.h"
#include "freestanding/memory/virtual.h"

void initMemoryManager(KernelMemory memory) {
    initPhysicalMemoryManager(memory.physical);
    initVirtualMemoryManager(memory.virt);
}

void *allocAndMap(U64 bytes) {
    //
    return allocPhysicalMemory(bytes);
}

void *allocContiguousAndMap(U64 bytes) {
    //
    return allocPhysicalMemory(bytes);
}

void freeMapped(Memory memory) {
    //
    freeMemory(memory);
}
