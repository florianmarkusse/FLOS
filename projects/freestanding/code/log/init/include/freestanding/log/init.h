#ifndef FREESTANDING_LOG_INIT_H
#define FREESTANDING_LOG_INIT_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/array-types.h"

extern U8_max_a loggingFlushBuffer;

void initLogger(Arena *perm);

#endif
