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

// These operations are only defined for powers of 2 !!!
U64 alignUp(U64 value, U64_pow2 align);
U64 alignDown(U64 value, U64_pow2 align);
U64 ceilingDivide(U64 value, U64_pow2 divisor);

U64 ringBufferIndex(U64 value, U64_pow2 ringBUfferSize);

U64 ringBufferIncrement(U64 value, U64_pow2 ringBUfferSize);
U64 ringBufferPlus(U64 value, U64 amount, U64_pow2 ringBUfferSize);

U64 ringBufferDecrement(U64 value, U64_pow2 ringBUfferSize);
U64 ringBufferMinus(U64 value, U64 amount, U64_pow2 ringBUfferSize);

// Moves value up to the closest power of 2. Unchanged if already a power of
// 2
U64_pow2 ceilingPowerOf2(U64 x);
bool isPowerOf2(U64 x);
U64 power(U64 base, Exponent exponent);
U64 divideByPowerOf2(U64 value, U64_pow2 divisor);

bool isAlignedTo(U64 x, U64_pow2 align);

#endif
