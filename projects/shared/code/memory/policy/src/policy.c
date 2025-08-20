#include "shared/memory/policy.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"

void *allocateIdentityMemory(U64 bytes, U64_pow2 align) {
    return allocPhysicalMemory(bytes, align);
}

void freeIdentityMemory(Memory memory) { freePhysicalMemory(memory); }

// NOTE: in subsequent iteration, we should only allow power of 2's for
// requests, minimum of virtual page size
void *allocateMappableMemory(U64 bytes, U64_pow2 align, U64_pow2 mappingSize) {
    ASSERT(isPowerOf2(align));
    ASSERT(isPowerOf2(mappingSize));
    ASSERT(mappingSize >= pageSizesSmallest());
    ASSERT(isAlignedTo(bytes, align));
    ASSERT(isAlignedTo(bytes, mappingSize));

    void *result = allocVirtualMemory(bytes, MAX(align, mappingSize));
    addPageMapping((Memory){.start = (U64)result, .bytes = bytes}, mappingSize);
    return result;
}

// TODO: This should be decided by the underlying architecture, realistically.
static constexpr auto MAX_PAGE_FLUSHES = 64;

void freeMappableMemory(Memory memory) {
    ASSERT(isAlignedTo(memory.start, pageSizesSmallest()));
    ASSERT(isAlignedTo(memory.bytes, pageSizesSmallest()));

    U64 virtualAddresses[MAX_PAGE_FLUSHES];
    U32 virtualAddressesLen = 0;

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
        for (typeof(virtualAddressesLen) i = 0; i < virtualAddressesLen; i++) {
            flushPageCacheEntry(virtualAddresses[i]);
        }
    } else {
        flushPageCache();
    }

    removePageMapping(memory.start);
    freeVirtualMemory(memory);
}
