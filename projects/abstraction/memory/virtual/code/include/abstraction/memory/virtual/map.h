#ifndef ABSTRACTION_MEMORY_VIRTUAL_MAP_H
#define ABSTRACTION_MEMORY_VIRTUAL_MAP_H

#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"

void mapVirtualRegionsWithFlags(U64 virt, PagedMemory memory, U64 pageSize,
                                U64 additionalFlags);
void mapVirtualRegions(U64 virt, PagedMemory memory, U64 pageSize);

void mapPage(U64 virt, U64 physical, U64 mappingSize);
void mapPageWithFlags(U64 virt, U64 physical, U64 mappingSize, U64 Flags);

#endif
