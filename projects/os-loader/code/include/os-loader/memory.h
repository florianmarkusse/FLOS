#ifndef OS_LOADER_MEMORY_BOOT_FUNCTIONS_H
#define OS_LOADER_MEMORY_BOOT_FUNCTIONS_H

#include "efi-to-kernel/kernel-parameters.h"
#include "efi/firmware/base.h" // for PhysicalAddress
#include "efi/firmware/memory.h"
#include "efi/memory/physical.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/sizes.h"
#include "shared/types/numeric.h" // for USize, U64, U32

static constexpr auto DYNAMIC_MEMORY_CAPACITY = 1 * MiB;

void allocateSpaceForKernelMemory(RedBlackNodeMM_max_a *nodes,
                                  RedBlackNodeMMPtr_max_a *freeList,
                                  Arena scratch);

void convertToKernelMemory(MemoryInfo *memoryInfo,
                           PackedMemoryAllocator *physicalMemoryTree,
                           RedBlackNodeMM_max_a *nodes,
                           RedBlackNodeMMPtr_max_a *freeList);

void findKernelMemory(U64 alignment, U64 numberOfThreads);

U64 alignVirtual(U64 virtualAddress, U64 physicalAddress, U64 bytes);

U64 mapMemory(U64 virt, U64 physical, U64 bytes, U64 flags);

#endif
