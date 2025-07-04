#ifndef X86_MEMORY_FLAGS_H
#define X86_MEMORY_FLAGS_H

#include "x86/memory/definitions.h"
#include "x86/memory/pat.h"

static constexpr auto STANDARD_PAGE_FLAGS =
    VirtualPageMasks.PAGE_PRESENT | VirtualPageMasks.PAGE_WRITABLE;
static constexpr auto GLOBAL_PAGE_FLAGS = VirtualPageMasks.PAGE_GLOBAL;
static constexpr auto SCREEN_MEMORY_PAGE_FLAGS = PATMapping.MAP_3;

#endif
