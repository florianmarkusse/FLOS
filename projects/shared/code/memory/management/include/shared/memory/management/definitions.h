#ifndef SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H
#define SHARED_MEMORY_MANAGEMENT_DEFINITIONS_H

#include "shared/types/array.h"
#include "shared/types/types.h"

typedef struct {
    U64 start;
    U64 numberOfPages;
} PagedMemory;

typedef MAX_LENGTH_ARRAY(PagedMemory) PagedMemory_max_a;
typedef ARRAY(PagedMemory) PagedMemory_a;

#endif
