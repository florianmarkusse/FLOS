#ifndef EFI_MEMORY_H
#define EFI_MEMORY_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/sizes.h"
#include "shared/types/types.h"

static constexpr U64 UEFI_PAGE_SIZE = 1 << 12;
static constexpr auto VIRTUAL_MEMORY_MAPPER_CAPACITY =
    (8 * MiB) / UEFI_PAGE_SIZE;
static constexpr auto VIRTUAL_MEMORY_MAPPER_ALIGNMENT = UEFI_PAGE_SIZE;

extern U64 bumpStartingAddress;
extern U64 bumpFreePages;

void freeBumpPages(U64 numPages);

#endif
