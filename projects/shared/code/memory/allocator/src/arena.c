#include "shared/memory/allocator/arena.h"
#include "abstraction/jmp.h"
#include "abstraction/memory/manipulation.h"
#include "shared/assert.h" // for ASSERT
#include "shared/memory/allocator/macros.h"
#include "shared/types/numeric.h"

__attribute__((malloc, alloc_align(3))) void *
alloc(Arena *a, U64 size, U64 align, U64 count, U8 flags) {
    ASSERT((align & (align - 1)) == 0);

    U64 avail = a->end - a->curFree;
    U64 padding = (-(U64)a->curFree) & (align - 1);
    if (count > (avail - padding) / size) {
        if (flags & NULLPTR_ON_FAIL) {
            return nullptr;
        }
        longjmp(a->jmpBuf, 1);
    }
    U64 total = size * count;
    U8 *p = a->curFree + padding;
    a->curFree += padding + total;

    return flags & ZERO_MEMORY ? memset(p, 0, total) : p;
}
