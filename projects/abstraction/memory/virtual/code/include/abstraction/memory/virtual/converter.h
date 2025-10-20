#ifndef ABSTRACTION_MEMORY_VIRTUAL_CONVERTER_H
#define ABSTRACTION_MEMORY_VIRTUAL_CONVERTER_H

#include "shared/types/numeric.h"

[[nodiscard]] U64 pageFlagsReadWrite();
[[nodiscard]] U64 pageFlagsNoCacheEvict();
[[nodiscard]] U64 pageFlagsScreenMemory();

[[nodiscard]] U64 pageSizesAvailableMask();
[[nodiscard]] U64_pow2 pageSizeSmallest();
[[nodiscard]] U64_pow2 pageSizeLargest();

#endif
