#ifndef EFI_MEMORY_H
#define EFI_MEMORY_H

#include "shared/memory/sizes.h"
#include "shared/types/types.h"

static constexpr U64 UEFI_PAGE_SIZE = 1 << 12;
static constexpr auto BUMP_ALLOCATOR_PAGE_INITIAL_CAPACITY =
    (1 * MiB) / UEFI_PAGE_SIZE;

extern U64 bumpStartingAddress;
extern U64 bumpFreePages;

void initBumpAllocator();
void freeBumpPages(U64 numPages);

#endif
