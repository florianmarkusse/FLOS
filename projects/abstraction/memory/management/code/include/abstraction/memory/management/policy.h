#ifndef ABSTRACTION_MEMORY_MANAGEMENT_POLICY_H
#define ABSTRACTION_MEMORY_MANAGEMENT_POLICY_H

#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"

void *allocAndMap(U64 bytes);
void *allocContiguousAndMap(U64 bytes);

void freeMapped(Memory memory);

#endif
