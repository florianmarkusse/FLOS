#include "shared/memory/policy.h"

#include "shared/memory/management/management.h"

void initMemoryManager(PackedKernelMemory memory) {
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
    freePhysicalMemory(memory);
}
