#include "efi/memory.h"

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

U64 bumpStartingAddress = 0;
U64 bumpFreePages = VIRTUAL_MEMORY_MAPPER_CAPACITY;

void initVirtualMemoryMapper(U64 address) { bumpStartingAddress = address; }

U64 allocate4KiBPages(U64 numPages) {
    ASSERT(bumpStartingAddress);

    if (bumpFreePages < numPages) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Not enough capacity in bump allocator!\n"));
        }
    }

    U64 address =
        bumpStartingAddress +
        ((VIRTUAL_MEMORY_MAPPER_CAPACITY - bumpFreePages) * UEFI_PAGE_SIZE);

    bumpFreePages -= numPages;

    return address;
}

void freeBumpPages(U64 numPages) { bumpFreePages += numPages; }
