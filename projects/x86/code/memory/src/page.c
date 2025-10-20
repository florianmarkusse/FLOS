#include "abstraction/memory/virtual/converter.h"
#include "x86/memory/definitions.h"

U64 pageSizesAvailableMask() {
    return (X86_4KIB_PAGE | X86_2MIB_PAGE | X86_1GIB_PAGE);
}
U64_pow2 pageSizeSmallest() {
    return 1ULL << (__builtin_ctzll(pageSizesAvailableMask()));
}
U64_pow2 pageSizeLargest() {
    return 1ULL << (((sizeof(U64) * BITS_PER_BYTE) - 1) -
                    (U64)__builtin_clzll(pageSizesAvailableMask()));
}
