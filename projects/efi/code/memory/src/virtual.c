#include "efi/memory/virtual.h"

#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi/error.h"
#include "efi/firmware/base.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "shared/log.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/types/types.h"

U64 virtualMemoryMapperFree = 0;
U64 virtualMemoryMapperEnd = 0;

void initVirtualMemoryMapper(U64 address) {
    virtualMemoryMapperFree = address;
    virtualMemoryMapperEnd = address + VIRTUAL_MEMORY_MAPPER_CAPACITY;
}

U64 getPageForMappingVirtualMemory() {
    ASSERT(virtualMemoryMapperFree);

    if (virtualMemoryMapperFree + VIRTUAL_MEMORY_MAPPER_SIZE >
        virtualMemoryMapperEnd) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Not enough capacity in virtual memory mapper!\n"));
        }
    }

    U64 result = virtualMemoryMapperFree;

    virtualMemoryMapperFree += VIRTUAL_MEMORY_MAPPER_SIZE;

    return result;
}
