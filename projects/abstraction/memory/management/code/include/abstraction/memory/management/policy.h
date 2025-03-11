#ifndef ABSTRACTION_MEMORY_MANAGEMENT_POLICY_H
#define ABSTRACTION_MEMORY_MANAGEMENT_POLICY_H

#include "shared/memory/converter.h"
#include "shared/types/types.h"
void *allocAndMapExplicit(Pages pages);
void *allocAndMap(U64 bytes);
void *allocContiguousAndMap(Pages pages);

void freeMapped(U64 start, U64 bytes);
U64 getVirtualMemory(U64 size, U64 alignValue);

#endif
