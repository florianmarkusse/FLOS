#ifndef ABSTRACTION_MEMORY_VIRTUAL_CONVERTER_H
#define ABSTRACTION_MEMORY_VIRTUAL_CONVERTER_H

#include "shared/types/numeric.h"

U64 pageFlagsReadWrite();
U64 pageFlagsNoCacheEvict();
U64 pageFlagsScreenMemory();

U64 pageSizesAvailableMask();
U64 pageSizesSmallest();
U64 pageSizesLargest();

#endif
