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
    // First is special
    Memory mapped = unmapPage(memory.start);

    // The virtual memory to free may be unaligned, make sure it is aligned
    // because we did remove aligned memory from the allocator.
    U64 virtualAddresses[MAX_PAGE_FLUSHES];
    U64 virtualAddressesLen = 0;

    U64 virtualPageStartAddress = ALIGN_DOWN_VALUE(memory.start, mapped.bytes);
    Memory toFreePhysical = {0};

    for (U64 endVirtualAddress = memory.start + memory.bytes;;) {
        if (mapped.start) {
            if (!toFreePhysical.start) {
                toFreePhysical = mapped;
            } else if (mapped.start ==
                       toFreePhysical.start + toFreePhysical.bytes) {
                toFreePhysical.bytes += mapped.bytes;
            } else {
                freePhysicalMemory(toFreePhysical);
                toFreePhysical = mapped; // Final free happens after loop
            }
        }

        if (virtualAddressesLen < MAX_PAGE_FLUSHES) {
            virtualAddresses[virtualAddressesLen] = virtualPageStartAddress;
            virtualAddressesLen++;
        }

        virtualPageStartAddress += mapped.bytes;
        if (virtualPageStartAddress >= endVirtualAddress) {
            break;
        }
        mapped = unmapPage(virtualPageStartAddress);
    }

    // Cleaup
    if (toFreePhysical.start) {
        freePhysicalMemory(toFreePhysical);
    }

    if (virtualAddressesLen < MAX_PAGE_FLUSHES) {
        for (U64 i = 0; i < virtualAddressesLen; i++) {
            flushPageCacheEntry(virtualAddresses[i]);
        }
    } else {
        flushPageCache();
    }

    freeVirtualMemory(
        (Memory){.start = virtualAddresses[0],
                 .bytes = virtualPageStartAddress - virtualAddresses[0]});
}
