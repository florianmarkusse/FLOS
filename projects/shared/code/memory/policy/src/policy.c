#include "shared/memory/policy.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"

void *allocateIdentityMemory(U64_pow2 blockSize) {
    ASSERT(blockSize >= pageSizesSmallest());
    ASSERT(isPowerOf2(blockSize));

    return allocPhysicalMemory(blockSize);
}

void freeIdentityMemory(Memory memory) {
    ASSERT(isAlignedTo(memory.start, pageSizesSmallest()));
    ASSERT(isAlignedTo(memory.bytes, pageSizesSmallest()));

    freePhysicalMemory(memory);
}

void freeIdentityMemoryNotBlockSize(Memory memory) {
    memory.start = alignUp(memory.start, pageSizesSmallest());
    memory.bytes = alignDown(memory.bytes, pageSizesSmallest());

    freePhysicalMemory(memory);
}

void *allocateMappableMemory(U64_pow2 blockSize, U64_pow2 mappingSize) {
    ASSERT(isPowerOf2(blockSize));
    ASSERT(blockSize >= pageSizesSmallest());
    ASSERT(isPowerOf2(mappingSize));
    ASSERT(mappingSize >= pageSizesSmallest());
    ASSERT(blockSize >= mappingSize);

    void *result = allocVirtualMemory(blockSize);
    addPageMapping((Memory){.start = (U64)result, .bytes = blockSize},
                   mappingSize);
    return result;
}

void freeMappableMemory(Memory memory) {
    ASSERT(isAlignedTo(memory.start, pageSizesSmallest()));
    ASSERT(isAlignedTo(memory.bytes, pageSizesSmallest()));

    U64 virtualAddresses[PAGE_CACHE_FLUSH_THRESHOLD];
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

        if (virtualAddressesLen < PAGE_CACHE_FLUSH_THRESHOLD) {
            virtualAddresses[virtualAddressesLen] = virtualPageStartAddress;
            virtualAddressesLen++;
        }
    }

    if (toFreePhysical.start) {
        freePhysicalMemory(toFreePhysical);
    }

    if (virtualAddressesLen < PAGE_CACHE_FLUSH_THRESHOLD) {
        for (typeof(virtualAddressesLen) i = 0; i < virtualAddressesLen; i++) {
            flushPageCacheEntry(virtualAddresses[i]);
        }
    } else {
        flushPageCache();
    }

    removePageMapping(memory.start);
    freeVirtualMemory(memory);
}
