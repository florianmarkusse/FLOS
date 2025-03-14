#ifndef SHARED_BUFFER_BUFFER_H
#define SHARED_BUFFER_BUFFER_H

#include "shared/types/array-types.h"
#include "shared/memory/allocator/arena.h"
#include "abstraction/text/converter/converter.h"
#include "shared/text/string.h"

void appendToSimpleBuffer(string data, U8_d_a *array, Arena *perm);

#define APPEND(data, buffer, perm)                                             \
    appendToSimpleBuffer(CONVERT_TO_STRING(data), buffer, perm)

#endif
