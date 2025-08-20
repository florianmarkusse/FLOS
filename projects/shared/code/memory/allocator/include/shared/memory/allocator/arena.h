#ifndef SHARED_MEMORY_ALLOCATOR_ARENA_H
#define SHARED_MEMORY_ALLOCATOR_ARENA_H

#include "abstraction/jmp.h"
#include "shared/macros.h"
#include "shared/types/numeric.h"

typedef struct {
    U8 *curFree;
    U8 *beg;
    U8 *end;
    JumpBuffer jmpBuf;
} Arena;

typedef struct {
    U64_pow2 align;
    U64 count;
    U8 flags;
} AllocParams;

__attribute__((malloc, alloc_align(3))) void *
alloc(Arena *a, U64 size, U64_pow2 align, U64 count, U8 flags);

#define NEW(a, t, ...)                                                         \
    ({                                                                         \
        AllocParams MACRO_VAR(allocParams) = (AllocParams){                    \
            .align = alignof(t), .count = 1, .flags = 0, __VA_ARGS__};         \
        (t *)alloc(a, sizeof(t), MACRO_VAR(allocParams).align,                 \
                   MACRO_VAR(allocParams).count,                               \
                   MACRO_VAR(allocParams).flags);                              \
    })

#endif
