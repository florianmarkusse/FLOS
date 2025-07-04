#include "shared/memory/policy.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/maths.h"
#include "shared/memory/management/management.h"

void initMemoryManager(PackedKernelMemory *memory) {
    initPhysicalMemoryManager(&memory->physical);
    initVirtualMemoryManager(&memory->virt);
}

void *allocateIdentityMemory(U64 bytes, U64 align) {
    return allocPhysicalMemory(bytes, align);
}

void freeIdentityMemory(Memory memory) { freePhysicalMemory(memory); }

// NOTE: have allocMappableMemory take into account the pageSize that will be
// used to map in case of page fault
void *allocateMappableMemory(U64 bytes, U64 align) {
    ASSERT(isPowerOf2(align));
    ASSERT(isAlignedTo(bytes, align));

    return allocVirtualMemory(bytes, align);
}

// TODO: This should be decided by the underlying architecture, realistically.
static constexpr auto MAX_PAGE_FLUSHES = 64;

void freeMappableMemory(Memory memory) {
    ASSERT(isAlignedTo(memory.start, SMALLEST_VIRTUAL_PAGE));
    ASSERT(isAlignedTo(memory.bytes, SMALLEST_VIRTUAL_PAGE));

    U64 virtualAddresses[MAX_PAGE_FLUSHES];
    U64 virtualAddressesLen = 0;

    Memory toFreePhysical = {0};
    Memory mapped;
    for (U64 virtualPageStartAddress = memory.start,
             endVirtualAddress = memory.start + memory.bytes;
         virtualPageStartAddress < endVirtualAddress;
         virtualPageStartAddress += mapped.bytes) {
        mapped = unmapPage(virtualPageStartAddress);
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
    }

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

    freeVirtualMemory(memory);
}
