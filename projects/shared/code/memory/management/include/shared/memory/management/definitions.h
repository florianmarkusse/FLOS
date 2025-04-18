#ifndef SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H
#define SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H

#include "shared/types/array.h"
#include "shared/types/types.h"

static constexpr U64 UEFI_PAGE_SIZE = 1 << 12;

typedef struct {
    U64 start;
    U64 bytes;
} Memory;

typedef ARRAY(Memory) Memory_a;

typedef struct {
    U64 start;
    U64 numberOfPages;
} PagedMemory;

typedef MAX_LENGTH_ARRAY(PagedMemory) PagedMemory_max_a;
typedef ARRAY(PagedMemory) PagedMemory_a;

#endif
