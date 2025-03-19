#ifndef SHARED_MEMORY_ALLOCATOR_ARENA_H
#define SHARED_MEMORY_ALLOCATOR_ARENA_H

#include "abstraction/jmp.h"
#include "shared/types/types.h"

typedef struct {
    U8 *curFree;
    U8 *beg;
    U8 *end;
    JumpBuffer jmp_buf;
} Arena;

__attribute((malloc, alloc_align(3))) void *alloc(Arena *a, U64 size, U64 align,
                                                  U64 count, U8 flags);

#define NEW_2(a, t) (t *)alloc(a, sizeof(t), alignof(t), 1, 0)
#define NEW_3(a, t, n) (t *)alloc(a, sizeof(t), alignof(t), n, 0)
#define NEW_4(a, t, n, f) (t *)alloc(a, sizeof(t), alignof(t), n, f)
#define NEW_5(a, t, n, f, align) (t *)alloc(a, sizeof(t), align, n, f)
#define NEW_X(a, b, c, d, e, f, ...) f
#define NEW(...)                                                               \
    NEW_X(__VA_ARGS__, NEW_5, NEW_4, NEW_3, NEW_2)                             \
    (__VA_ARGS__)

#endif
