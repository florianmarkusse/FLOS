#include "shared/maths/maths.h"
#include "shared/types/types.h"

U64 ceilingPowerOf2(U64 x) {
    return x == 1 ? 1 : 1 << (64 - __builtin_clzll(x - 1));
}

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
