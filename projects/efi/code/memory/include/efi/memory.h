#ifndef EFI_MEMORY_H
#define EFI_MEMORY_H

#include "shared/types/types.h"
static constexpr U64 UEFI_PAGE_SIZE = 1 << 12;
static constexpr auto BUMP_ALLOCATOR_PAGE_INITIAL_CAPACITY = 512;

extern U64 bumpStartingAddress;
extern U64 bumpFreePages;

void initBumpAllocator();

#endif
