#ifndef SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H
#define SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H

#include "shared/types/array.h"
#include "shared/types/numeric.h"

static constexpr U64 UEFI_PAGE_SIZE = 1 << 12;

typedef struct {
    U64 start;
    U64 bytes;
} Memory;

typedef MAX_LENGTH_ARRAY(Memory) Memory_max_a;
typedef ARRAY(Memory) Memory_a;

typedef struct {
    U64 start;
    U64 end; // exclusive, naturally.
} MemoryRange;

typedef MAX_LENGTH_ARRAY(MemoryRange) MemoryRange_max_a;
typedef ARRAY(MemoryRange) MemoryRange_a;

#endif
