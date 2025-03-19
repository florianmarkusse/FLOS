#ifndef EFI_MEMORY_H
#define EFI_MEMORY_H

#include "abstraction/memory/virtual/converter.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/sizes.h"
#include "shared/types/types.h"

static constexpr auto VIRTUAL_MEMORY_MAPPER_PAGES = 16;
static constexpr auto VIRTUAL_MEMORY_MAPPER_CAPACITY =
    VIRTUAL_MEMORY_MAPPER_PAGES * VIRTUAL_MEMORY_MAPPER_SIZE;
static constexpr auto VIRTUAL_MEMORY_MAPPER_ALIGNMENT =
    VIRTUAL_MEMORY_MAPPER_SIZE;

extern U64 virtualMemoryMapperFree;
extern U64 virtualMemoryMapperEnd;

void initVirtualMemoryMapper(U64 address);

#endif
