#ifndef SHARED_MEMORY_CONVERTER_H
#define SHARED_MEMORY_CONVERTER_H

#include "shared/types/numeric.h"

// Check if you are not at the largest page size first!
[[nodiscard]] U64_pow2 pageSizeIncrease(U64_pow2 pageSize);
// Check if you are not at the smallest page size first!
[[nodiscard]] U64_pow2 pageSizeDecrease(U64_pow2 pageSize);

// Returns max page size if too big
[[nodiscard]] U64_pow2 pageSizeEncompassing(U64 bytes);
// Returns max page size if too big
[[nodiscard]] U64_pow2 pageSizeLeastLargerThan(U64 address, U64 bytes);
[[nodiscard]] U64_pow2 pageSizeFitting(U64 bytes);

#endif
