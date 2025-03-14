#ifndef ABSTRACTION_MEMORY_PHYSICAL_ALLOCATION_H
#define ABSTRACTION_MEMORY_PHYSICAL_ALLOCATION_H

#include "shared/types/types.h"

// TODO: Rename this and remove parameter, should just always be "1"
U64 allocate4KiBPages(U64 numPages);

#endif
