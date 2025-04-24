#ifndef SHARED_TYPES_COMMON_H
#define SHARED_TYPES_COMMON_H

#include "shared/types/array.h"
#include "shared/types/numeric.h"

typedef struct {
    U64 start;
    U64 end; // exclusive, naturally.
} Range;

typedef MAX_LENGTH_ARRAY(Range) Range_max_a;
typedef ARRAY(Range) Range_a;

#endif
