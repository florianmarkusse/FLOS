#ifndef SHARED_MATHS_H
#define SHARED_MATHS_H

#include "shared/types/numeric.h"

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

[[nodiscard]] U64 alignUp(U64 value, U64_pow2 align);
[[nodiscard]] U64 alignDown(U64 value, U64_pow2 align);
[[nodiscard]] U64 ceilingDivide(U64 value, U64_pow2 divisor);

[[nodiscard]] U64 ringBufferIndex(U64 value, U64_pow2 ringBUfferSize);

[[nodiscard]] U64 ringBufferIncrement(U64 value, U64_pow2 ringBUfferSize);
[[nodiscard]] U64 ringBufferPlus(U64 value, U64 amount,
                                 U64_pow2 ringBUfferSize);

[[nodiscard]] U64 ringBufferDecrement(U64 value, U64_pow2 ringBUfferSize);
[[nodiscard]] U64 ringBufferMinus(U64 value, U64 amount,
                                  U64_pow2 ringBUfferSize);

// Moves value up to the closest power of 2. Unchanged if already a power of
// 2
[[nodiscard]] U64_pow2 ceilingPowerOf2(U64 x);
// Moves value down to the closest power of 2. Unchanged if already a power of
// 2
[[nodiscard]] U64_pow2 floorPowerOf2(U64 x);
[[nodiscard]] bool isPowerOf2(U64 x);
[[nodiscard]] U64 power(U64 base, Exponent exponent);
[[nodiscard]] U64 divideByPowerOf2(U64 value, U64_pow2 divisor);

[[nodiscard]] bool isAlignedTo(U64 x, U64_pow2 align);

#endif
