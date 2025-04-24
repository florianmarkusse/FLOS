#ifndef SHARED_MEMORY_CONVERTER_H
#define SHARED_MEMORY_CONVERTER_H

#include "shared/types/numeric.h"

// Check if you are not at the largest page size first!
U64 increasePageSize(U64 pageSize);
// Check if you are not at the smallest page size first!
U64 decreasePageSize(U64 pageSize);

// Returns max page size if too big
U64 pageSizeEncompassing(U64 bytes);
// Returns max page size if too big
U64 pageSizeLeastLargerThan(U64 address, U64 bytes);
// Returns max page size if too big
U64 pageSizeFitting(U64 address, U64 bytes);

#endif
