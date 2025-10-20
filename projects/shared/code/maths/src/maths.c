#include "shared/maths.h"
#include "shared/types/numeric.h"

U64 alignUp(U64 value, U64_pow2 align) {
    return (value + align - 1) & (~(align - 1));
}

U64 alignDown(U64 value, U64_pow2 align) { return (value) & (~(align - 1)); }

U64 ceilingDivide(U64 value, U64_pow2 divisor) {
    int shift = __builtin_ctzll(divisor);
    return (value + divisor - 1) >> shift;
}

U64 ringBufferIndex(U64 value, U64_pow2 ringBUfferSize) {
    return value & (ringBUfferSize - 1);
}

U64 ringBufferIncrement(U64 value, U64_pow2 ringBUfferSize) {
    return ringBufferIndex(value + 1, ringBUfferSize);
}

U64 ringBufferPlus(U64 value, U64 amount, U64_pow2 ringBUfferSize) {
    return ringBufferIndex(value + amount, ringBUfferSize);
}

U64 ringBufferDecrement(U64 value, U64_pow2 ringBUfferSize) {
    return ringBufferIndex(value - 1, ringBUfferSize);
}

U64 ringBufferMinus(U64 value, U64 amount, U64_pow2 ringBUfferSize) {
    return ringBufferIndex(value - amount, ringBUfferSize);
}

U64_pow2 ceilingPowerOf2(U64 x) {
    if (x <= 1) {
        return 1;
    }
    return 1ULL << ((sizeof(U64) * BITS_PER_BYTE) -
                    (U64)__builtin_clzll(x - 1));
}

U64_pow2 floorPowerOf2(U64 x) {
    if (!x) {
        return 0;
    }

    return 1ULL << (((sizeof(U64) * BITS_PER_BYTE) - 1) -
                    (U64)__builtin_clzll(x));
}
bool powerOf2(U64 x) { return (x & (x - 1)) == 0; }

U64 power(U64 base, Exponent exponent) {
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

U64 dividePowerOf2(U64 value, U64_pow2 divisor) {
    return value >> (__builtin_ctzll(divisor));
}

bool aligned(U64 x, U64_pow2 align) { return (x & (align - 1)) == 0; }
