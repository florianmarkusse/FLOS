#include "shared/hash/msi/common.h"

#include "abstraction/memory/manipulation.h"
#include "shared/assert.h" // for ASSERT
#include "shared/memory/allocator/macros.h"
#include "shared/types/numeric.h"

/**
 * Written assuming that arena bumps up! Otherwise the middle case statement
 * where we only do a times 1 alloc does not hold.
 */
void MSISetNew(void *setSlice, U64 size, U64_pow2 align, Arena *a) {
    SetSlice *replica = (SetSlice *)setSlice;
    ASSERT(replica->exp > 0);

    if (replica->exp >= 31) {
        ASSERT(false);
        longjmp(a->jmpBuf, 1);
    }

    U64 cap = 1 << replica->exp;

    if (replica->buf == nullptr) {
        replica->buf = alloc(a, size, align, cap, ZERO_MEMORY);
    } else if (a->beg == replica->buf + size * cap) {
        memset(replica->buf, 0, size * cap);
        (void)alloc(a, size, 1, cap, ZERO_MEMORY);
        replica->exp++;
        replica->len = 0;
    } else {
        void *data = alloc(a, 2 * size, align, cap, ZERO_MEMORY);
        replica->buf = data;
        replica->exp++;
        replica->len = 0;
    }
}

U32 indexLookup(U64 hash, U16 exp, U32 idx) {
    U32 mask = ((U32)1 << exp) - 1;
    U32 step = (U32)(hash >> (64 - exp)) | 1;
    return (idx + step) & mask;
}
