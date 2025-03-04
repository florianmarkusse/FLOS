#include "efi/memory.h"

#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/memory/descriptor.h"
#include "efi/error.h"
#include "efi/firmware/base.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "shared/text/string.h"
#include "shared/types/types.h"

U64 bumpStartingAddress = 0;
U64 bumpFreePages = BUMP_ALLOCATOR_PAGE_INITIAL_CAPACITY;

void initBumpAllocator() {
    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA, BUMP_ALLOCATOR_PAGE_INITIAL_CAPACITY,
        &bumpStartingAddress);

    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could now initialize bump allocator"));
    }
}

U64 allocate4KiBPages(U64 numPages) {
    ASSERT(bumpStartingAddress);

    if (bumpFreePages < numPages) {
        EXIT_WITH_MESSAGE {
            ERROR(STRING("Not enough capacity in bump allocator!\n"));
        }
    }

    U64 address = bumpStartingAddress +
                  ((BUMP_ALLOCATOR_PAGE_INITIAL_CAPACITY - bumpFreePages) *
                   UEFI_PAGE_SIZE);
    bumpFreePages -= numPages;

    return address;
}
