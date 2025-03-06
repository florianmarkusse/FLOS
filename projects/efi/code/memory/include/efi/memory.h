#ifndef EFI_MEMORY_H
#define EFI_MEMORY_H

#include "shared/memory/sizes.h"
#include "shared/types/types.h"

static constexpr U64 UEFI_PAGE_SIZE = 1 << 12;
static constexpr auto BUMP_ALLOCATOR_PAGE_INITIAL_CAPACITY =
    (64 * MiB) / UEFI_PAGE_SIZE;

extern U64 bumpStartingAddress;
extern U64 bumpFreePages;

#define ALLOCATE_WITH_ORIGIN(origin, numPages)                                 \
    ({                                                                         \
        KFLUSH_AFTER {                                                         \
            INFO(STRING("Allocating "));                                       \
            INFO(numPages);                                                    \
            INFO(STRING(" from "));                                            \
            INFO(origin, NEWLINE);                                             \
        }                                                                      \
        allocate4KiBPages(numPages);                                           \
    })

void initBumpAllocator();
void freeBumpPages(U64 numPages);

#endif
