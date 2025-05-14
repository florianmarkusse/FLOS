#include "abstraction/memory/physical.h"

#include "shared/memory/management/management.h"
#include "shared/types/numeric.h"

U64 getPageForMappingVirtualMemory(U64 pageSize, U64 align) {
    return (U64)allocPhysicalMemory(pageSize, align);
}
