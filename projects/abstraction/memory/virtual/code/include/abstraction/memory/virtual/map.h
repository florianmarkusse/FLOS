#ifndef ABSTRACTION_MEMORY_VIRTUAL_MAP_H
#define ABSTRACTION_MEMORY_VIRTUAL_MAP_H

#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

void mapPage(U64 virt, U64 physical, U64 mappingSize);
void mapPageWithFlags(U64 virt, U64 physical, U64 mappingSize, U64 Flags);

// Unmaps the virtual address space and returns the physical memory that can now
// freely be used.
Memory unmapPage(U64 virt);

Memory flushPageCacheEntry(U64 virt);
Memory flushPageCache();

#endif
