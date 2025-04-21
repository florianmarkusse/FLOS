#include "abstraction/memory/physical/allocation.h"

#include "abstraction/memory/virtual/converter.h"
#include "efi/memory/physical.h"
#include "shared/types/types.h"

U64 getPageForMappingVirtualMemory(U64 pageSize) {
    return allocateUnalignedMemory(pageSize, true);
}
