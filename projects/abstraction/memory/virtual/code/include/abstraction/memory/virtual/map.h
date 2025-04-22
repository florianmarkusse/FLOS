#ifndef ABSTRACTION_MEMORY_VIRTUAL_MAP_H
#define ABSTRACTION_MEMORY_VIRTUAL_MAP_H

#include "shared/types/types.h"

void setRootPageTable();

void mapPage(U64 virt, U64 physical, U64 mappingSize);
void mapPageWithFlags(U64 virt, U64 physical, U64 mappingSize, U64 Flags);

#endif
