#include "abstraction/memory/physical.h"

#include "efi/memory/physical.h"
#include "shared/types/types.h"

U64 getPageForMappingVirtualMemory(U64 pageSize) {
    return allocateUnalignedMemory(pageSize, true);
}
