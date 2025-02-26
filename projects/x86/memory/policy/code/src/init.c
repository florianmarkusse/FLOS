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

void initScreenMemory(U64 screenAddress, U64 bytes) {
    PagedMemory pagedMemory = {.pageStart = screenAddress,
                               .numberOfPages =
                                   CEILING_DIV_EXP(bytes, PAGE_FRAME_SHIFT)};
    mapVirtualRegionWithFlags(screenAddress, pagedMemory, BASE_PAGE,
                              PATMapping.MAP_3);
    flushCPUCaches();
}
