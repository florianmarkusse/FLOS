#ifndef SHARED_HASH_HASHES_H
#define SHARED_HASH_HASHES_H

#include "shared/types/numeric.h"

#include "shared/text/string.h"

[[nodiscard]] U64 hashStringSkeeto(String string);

// https://github.com/skeeto/hash-prospector
[[nodiscard]] U32 hashU32(U32 x);

// https://github.com/skeeto/hash-prospector
// 3-round xorshift-multiply (-Xn3)
// bias = 0.0045976709018820602
[[nodiscard]] U16 hashU16(U16 x);

#endif
