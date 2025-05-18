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

static constexpr auto MAX_PAGE_FLUSHES = 64;

void freeMappableMemory(Memory memory) {
    KFLUSH_AFTER {
        INFO(STRING("Freeing mappable[start="));
        INFO((void *)memory.start);
        INFO(STRING(", bytes="));
        INFO(memory.bytes);
        INFO(STRING("]\n"));
    }

    Memory mappedAddress = unmapPage(memory.start);
    Memory toFreePhysical = mappedAddress;

    U64 virtualAddresses[MAX_PAGE_FLUSHES];
    U64 virtualAddressesLen = 0;

    U64 currentAddress = ALIGN_DOWN_VALUE(memory.start, mappedAddress.bytes);
    for (U64 endAddress = memory.start + memory.bytes;
         currentAddress < endAddress;
         mappedAddress = unmapPage(currentAddress)) {
        KFLUSH_AFTER {
            INFO(STRING("found mappable[start="));
            INFO((void *)mappedAddress.start);
            INFO(STRING(", bytes="));
            INFO(mappedAddress.bytes);
            INFO(STRING("]\n"));
        }

        if (mappedAddress.start) {
            if (mappedAddress.start ==
                toFreePhysical.start + toFreePhysical.bytes) {
                toFreePhysical.bytes += mappedAddress.bytes;
            } else {
                freePhysicalMemory(toFreePhysical);
                toFreePhysical = mappedAddress; // Final free happens after loop
            }
        }

        if (virtualAddressesLen < MAX_PAGE_FLUSHES) {
            virtualAddresses[virtualAddressesLen] = currentAddress;
            virtualAddressesLen++;
        }

        currentAddress += mappedAddress.bytes;
    }
    freePhysicalMemory(toFreePhysical);

    if (virtualAddressesLen >= MAX_PAGE_FLUSHES) {
        for (U64 i = 0; i < virtualAddressesLen; i++) {
            flushPageCacheEntry(virtualAddresses[i]);
        }
    } else {
        flushPageCache();
    }

    freeVirtualMemory((Memory){.start = virtualAddresses[0],
                               .bytes = currentAddress - virtualAddresses[0]});

    // NOTE: Not freeing the virtual memory (yet) , because we are not sure yet
    // how to invalidate the tlb with the memory that we freed
}
