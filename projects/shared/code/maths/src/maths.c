#include "shared/maths.h"
#include "shared/types/numeric.h"

U64 alignUp(U64 value, U64 align) {
    return (value + align - 1) & (~(align - 1));
}

U64 alignDown(U64 value, U64 align) { return (value) & (~(align - 1)); }

U64 ceilingDivide(U64 value, U64 divisor) {
    int shift = __builtin_ctzll(divisor);
    return (value + divisor - 1) >> shift;
}

U64 ceilingPowerOf2(U64 x) {
    if (x <= 1) {
        return 1;
    }
    return 1ULL << ((sizeof(U64) * BITS_PER_BYTE) -
                    (U64)__builtin_clzll(x - 1));
}

bool isPowerOf2(U64 x) { return (x & (x - 1)) == 0; }

U64 power(U64 base, U64 exponent) {
    U64 result = 1;

    while (exponent > 0) {
        if (exponent & 1) {
            if (result > U64_MAX / base) {
                return 0;
            }
            result *= base;
            if (exponent == 1) {
                return result;
            }
        }
        if (base > U64_MAX / base) {
            return 0;
        }
        base *= base;
        exponent >>= 1;
    }

    return result;
}

U64 divideByPowerOf2(U64 value, U64 divisor) {
    return value >> (__builtin_ctzll(divisor));
}

bool isAlignedTo(U64 x, U64 align) { return (x & (align - 1)) == 0; }
