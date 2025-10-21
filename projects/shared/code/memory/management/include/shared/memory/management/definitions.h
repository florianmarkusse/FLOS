#ifndef SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H
#define SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H

#include "shared/types/array.h"
#include "shared/types/numeric.h"

typedef struct {
    U64 start;
    U64 bytes;
} Memory;

typedef ARRAY_MAX_LENGTH(Memory) Memory_max_a;
typedef ARRAY(Memory) Memory_a;

#endif
