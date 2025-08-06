#ifndef ABSTRACTION_MEMORY_VIRTUAL_CONVERTER_H
#define ABSTRACTION_MEMORY_VIRTUAL_CONVERTER_H

#include "shared/types/numeric.h"
U64 pageFlagsReadWrite();
U64 pageFlagsNoCacheEvict();
U64 pageFlagsScreenMemory();

#ifdef X86
#include "x86/memory/definitions.h"
#else
#error ABSTRACTION_MEMORY_VIRTUAL_CONVERTER_H
#endif

#endif
