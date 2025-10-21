#ifndef SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H
#define SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H

#include "shared/memory/allocator/buddy.h"
#include "shared/memory/allocator/node.h"

static constexpr auto BUDDY_BLOCKS_CAPACITY_PER_ORDER_DEFAULT = 512;

extern Buddy buddyPhysical;
static constexpr auto BUDDY_PHYSICAL_PAGE_SIZE_MAX = 30;

extern Buddy buddyVirtual;
static constexpr auto BUDDY_VIRTUAL_PAGE_SIZE_MAX = 57;

[[nodiscard]] void *virtualMemoryAlloc(U64_pow2 blockSize);
void virtualMemoryFree(Memory memory);

[[nodiscard]] void *physicalMemoryAlloc(U64_pow2 blockSize);
void physicalMemoryFree(Memory memory);

#endif
