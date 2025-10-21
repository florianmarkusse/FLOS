#ifndef SHARED_TYPES_ARRAY_TYPES_H
#define SHARED_TYPES_ARRAY_TYPES_H

#include "shared/types/array.h"
#include "shared/types/numeric.h"

typedef ARRAY_MAX_LENGTH(U64) U64_max_a;
typedef ARRAY_MAX_LENGTH(U32) U32_max_a;
typedef ARRAY_MAX_LENGTH(U8) U8_max_a;
typedef ARRAY_MAX_LENGTH(bool) bool_max_a;
typedef ARRAY_MAX_LENGTH(void) void_max_a;
typedef ARRAY_MAX_LENGTH(void *) voidPtr_max_a;
typedef ARRAY(void) void_a;
typedef ARRAY(U8) U8_a;
typedef ARRAY(U64) U64_a;
typedef ARRAY(U32) U32_a;

#endif
