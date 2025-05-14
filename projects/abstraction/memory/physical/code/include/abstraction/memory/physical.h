#ifndef ABSTRACTION_MEMORY_PHYSICAL_H
#define ABSTRACTION_MEMORY_PHYSICAL_H

#include "shared/types/numeric.h"

U64 getPageForMappingVirtualMemory(U64 pageSize, U64 align);

#endif
