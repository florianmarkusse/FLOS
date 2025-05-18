#include "shared/memory/policy.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/management.h"

void initMemoryManager(PackedKernelMemory memory) {
    initPhysicalMemoryManager(memory.physical);
    initVirtualMemoryManager(memory.virt);
}

void *allocateIdentityMemory(U64 bytes) {
    return allocPhysicalMemory(bytes, 1);
}

void freeIdentityMemory(Memory memory) { freePhysicalMemory(memory); }

void *allocateMappableMemory(U64 bytes, U64 align) {
    return allocVirtualMemory(ALIGN_UP_VALUE(bytes, SMALLEST_VIRTUAL_PAGE),
                              ALIGN_UP_VALUE(align, SMALLEST_VIRTUAL_PAGE));
}

void freeMappableMemory(Memory memory) {
    KFLUSH_AFTER {
        INFO(STRING("Freeing mappable[start="));
        INFO((void *)memory.start);
        INFO(STRING(", bytes="));
        INFO(memory.bytes);
        INFO(STRING("]\n"));
    }

    Memory mappedAddress = unmapPage(memory.start);

    U64 startingVirtualAddress =
        ALIGN_DOWN_VALUE(memory.start, mappedAddress.bytes);
    U64 currentAddress = startingVirtualAddress;
    U64 endAddress = memory.start + memory.bytes;
    for (; currentAddress < endAddress;
         mappedAddress = unmapPage(currentAddress)) {
        KFLUSH_AFTER {
            INFO(STRING("found mappable[start="));
            INFO((void *)mappedAddress.start);
            INFO(STRING(", bytes="));
            INFO(mappedAddress.bytes);
            INFO(STRING("]\n"));
        }

        if (mappedAddress.start) {
            freePhysicalMemory(mappedAddress);
        }
        currentAddress += mappedAddress.bytes;
    }

    freeVirtualMemory(
        (Memory){.start = startingVirtualAddress,
                 .bytes = currentAddress - startingVirtualAddress});

    // NOTE: Not freeing the virtual memory (yet) , because we are not sure yet
    // how to invalidate the tlb with the memory that we freed
}
