#ifndef X86_MEMORY_FLAGS_H
#define X86_MEMORY_FLAGS_H

#include "x86/memory/definitions.h"
#include "x86/memory/pat.h"

U64 pageFlagsReadWrite() {
    return VirtualPageMasks.PAGE_PRESENT | VirtualPageMasks.PAGE_WRITABLE;
}
U64 pageFlagsNoCacheEvict() { return VirtualPageMasks.PAGE_GLOBAL; }
U64 pageFlagsScreenMemory() { return PATMapping.MAP_3; }

#endif
