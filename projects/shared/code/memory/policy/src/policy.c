#include "shared/memory/policy.h"

#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/memory/management/management.h"
#include "shared/memory/management/page.h"

void *allocateIdentityMemory(U64_pow2 blockSize) {
    ASSERT(blockSize >= pageSizeSmallest());
    ASSERT(powerOf2(blockSize));

    return allocPhysicalMemory(blockSize);
}

void freeIdentityMemory(Memory memory) {
    ASSERT(aligned(memory.start, pageSizeSmallest()));
    ASSERT(aligned(memory.bytes, pageSizeSmallest()));

    freePhysicalMemory(memory);
}

void freeIdentityMemoryNotBlockSize(Memory memory) {
    memory.start = alignUp(memory.start, pageSizeSmallest());
    memory.bytes = alignDown(memory.bytes, pageSizeSmallest());

    freePhysicalMemory(memory);
}

void *allocateMappableMemory(U64_pow2 blockSize, U64_pow2 mappingSize) {
    ASSERT(powerOf2(blockSize));
    ASSERT(blockSize >= pageSizeSmallest());
    ASSERT(powerOf2(mappingSize));
    ASSERT(mappingSize >= pageSizeSmallest());
    ASSERT(blockSize >= mappingSize);

    void *result = allocVirtualMemory(blockSize);
    addPageMapping((Memory){.start = (U64)result, .bytes = blockSize},
                   mappingSize);
    return result;
}

void freeMappableMemory(Memory memory) {
    ASSERT(aligned(memory.start, pageSizeSmallest()));
    ASSERT(aligned(memory.bytes, pageSizeSmallest()));

    U64 virtualAddresses[PAGE_CACHE_FLUSH_THRESHOLD];
    U32 virtualAddressesLen = 0;

    Memory toFreePhysical = {0};
    Memory mapped;
    for (U64 virtualPageStartAddress = memory.start,
             endVirtualAddress = memory.start + memory.bytes;
         virtualPageStartAddress < endVirtualAddress;
         virtualPageStartAddress += mapped.bytes) {
        mapped = pageUnmap(virtualPageStartAddress);
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
            pageCacheEntryFlush(virtualAddresses[i]);
        }
    } else {
        pageCacheFlush();
    }

    removePageMapping(memory.start);
    freeVirtualMemory(memory);
}
