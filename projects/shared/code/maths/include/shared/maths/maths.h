#ifndef SHARED_MATHS_MATHS_H
#define SHARED_MATHS_MATHS_H

#include "shared/types/types.h"

#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))

#define MIN3(a, b, c) MIN2(MIN2(a, b), c)
#define MAX3(a, b, c) MAX2(MAX2(a, b), c)

#define MIN4(a, b, c, d) MIN2(MIN3(a, b, c), d)
#define MAX4(a, b, c, d) MAX2(MAX3(a, b, c), d)

#define MIN(...) MIN_N(__VA_ARGS__, MIN4, MIN3, MIN2)(__VA_ARGS__)
#define MAX(...) MAX_N(__VA_ARGS__, MAX4, MAX3, MAX2)(__VA_ARGS__)

#define MIN_N(_1, _2, _3, _4, NAME, ...) NAME
#define MAX_N(_1, _2, _3, _4, NAME, ...) NAME

#define ABS(x) (((x) < 0) ? (-(x)) : (x))

// These operations are only defined for powers of 2 !!!
#define ALIGN_UP_EXP(val, exponent)                                            \
    (((val) + ((TYPED_CONSTANT(val, 1) << (exponent)) - 1)) &                  \
     (~((TYPED_CONSTANT(val, 1) << (exponent)) - 1)))
#define ALIGN_UP_VALUE(val, alignValue)                                        \
    (((val) + ((alignValue) - 1)) & (~((alignValue) - 1)))
#define ALIGN_DOWN_EXP(val, exponent)                                          \
    ((val) & (~((TYPED_CONSTANT(val, 1) << (exponent)) - 1)))
#define ALIGN_DOWN_VALUE(val, alignValue) ((val) & (~((alignValue) - 1)))
#define CEILING_DIV_EXP(val, exponent)                                         \
    (((val) + ((TYPED_CONSTANT(val, 1) << (exponent)) - 1)) >> (exponent))
#define CEILING_DIV_VALUE(val, divisor)                                        \
    ({                                                                         \
        typeof(divisor) _d = (divisor);                                        \
        typeof(val) _v = (val);                                                \
        int shift = _Generic((divisor),                                        \
            U32: __builtin_ctz(_d),                                            \
            U64: __builtin_ctzll(_d));                                         \
        ((_v + _d - 1) >> shift);                                              \
    })

#define RING_RANGE_EXP(val, exponent)                                          \
    ((val) & ((TYPED_CONSTANT(val, 1) << (exponent)) - 1))
#define RING_RANGE_VALUE(val, ringSize) (((val)) & ((ringSize) - 1))
#define RING_INCREMENT(val, ringSize) (((val) + 1) & ((ringSize) - 1))
#define RING_PLUS(val, amount, ringSize) (((val) + (amount)) & ((ringSize) - 1))
#define RING_DECREMENT(val, ringSize) (((val) - 1) & ((ringSize) - 1))
#define RING_MINUS(val, amount, ringSize)                                      \
    (((val) - (amount)) & ((ringSize) - 1))

// Moves value up to the closest power of 2. Unchanged if already a power of 2
U64 ceilingPowerOf2(U64 x);

U64 power(U64 base, U64 exponent);

#endif
