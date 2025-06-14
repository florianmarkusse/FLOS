#ifndef ABSTRACTION_MEMORY_VIRTUAL_ALLOCATOR_H
#define ABSTRACTION_MEMORY_VIRTUAL_ALLOCATOR_H

#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

typedef enum {
    VIRTUAL_PAGE_TABLE_ALLOCATION,
    META_DATA_PAGE_ALLOCATION,
} VirtualAllocationType;

void *getZeroedMemoryForVirtual(VirtualAllocationType type);
void freeZeroedMemoryForVirtual(U64 address, VirtualAllocationType type);

#endif
