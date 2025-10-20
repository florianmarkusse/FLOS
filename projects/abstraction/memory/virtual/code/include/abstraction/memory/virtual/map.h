#ifndef ABSTRACTION_MEMORY_VIRTUAL_MAP_H
#define ABSTRACTION_MEMORY_VIRTUAL_MAP_H

#include "abstraction/memory/virtual/converter.h"
#include "shared/macros.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

typedef struct {
    U64 flags;
} MappingParams;

void pageMap_(U64 virt, U64 physical, U64_pow2 mappingSize, U64 flags);

#define mapPage(virt, physical, mappingSize, ...)                              \
    ({                                                                         \
        MappingParams MACRO_VAR(mappingParams) =                               \
            (MappingParams){.flags = pageFlagsReadWrite(), __VA_ARGS__};       \
        pageMap_(virt, physical, mappingSize, MACRO_VAR(mappingParams).flags); \
    })

// Unmaps the virtual address space and returns the physical memory that can now
// freely be used. If nothing was mapped to the address, returns address of 0
// with bytes being the size of that page which is unmapped.
[[nodiscard]] Memory pageUnmap(U64 virt);

void pageCacheEntryFlush(U64 virt);
void pageCacheFlush();

// NOTE: c26 please consteva0 or smth

#ifdef X86
#define PAGE_CACHE_FLUSH_THRESHOLD 32
#else
#error ABSTRACTION_MEMORY_VIRTUAL_MAP_H
#endif

#endif
