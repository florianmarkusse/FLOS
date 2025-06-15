#ifndef ABSTRACTION_MEMORY_VIRTUAL_MAP_H
#define ABSTRACTION_MEMORY_VIRTUAL_MAP_H

#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

void mapPage(U64 virt, U64 physical, U64 mappingSize);
void mapPageWithFlags(U64 virt, U64 physical, U64 mappingSize, U64 Flags);

// Unmaps the virtual address space and returns the physical memory that can now
// freely be used. If nothing was mapped to the address, returns address of 0
// with bytes being the size of that page which is unmapped.
Memory unmapPage(U64 virt);

void flushPageCacheEntry(U64 virt);
void flushPageCache();

U64 getVirtualMemoryMappingBytes();
U64 getVirtualMemoryMappingAlignment();

U64 getVirtualMemoryMetaDataBytes();
U64 getVirtualMemoryMetaDataAlignment();

#endif
