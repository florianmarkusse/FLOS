#include "abstraction/memory/management/init.h"

#include "abstraction/memory/virtual/map.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/definitions.h"
#include "x86/configuration/cpu.h"
#include "x86/memory/definitions.h"
#include "x86/memory/pat.h"
#include "x86/memory/physical.h"
#include "x86/memory/policy/virtual.h"

void initMemoryManager(KernelMemory kernelMemory) {
    initPhysicalMemoryManager(kernelMemory);
    initVirtualMemoryManager(kernelMemory);
}

U64 initScreenMemory(U64 physicalScreenAddress, U64 bytes) {
    PagedMemory pagedMemory = {.start = physicalScreenAddress,
                               .numberOfPages =
                                   CEILING_DIV_VALUE(bytes, X86_2MIB_PAGE)};
    U64 virtualMemory = getVirtualMemory(bytes, X86_2MIB_PAGE);
    mapVirtualRegionWithFlags(virtualMemory, pagedMemory, X86_2MIB_PAGE,
                              PATMapping.MAP_3);
    flushCPUCaches();

    return virtualMemory;
}
