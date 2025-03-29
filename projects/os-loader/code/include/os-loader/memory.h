#ifndef OS_LOADER_MEMORY_BOOT_FUNCTIONS_H
#define OS_LOADER_MEMORY_BOOT_FUNCTIONS_H

#include "efi-to-kernel/kernel-parameters.h"
#include "efi/firmware/base.h" // for PhysicalAddress
#include "efi/firmware/memory.h"
#include "efi/memory/definitions.h"
#include "efi/memory/physical.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/sizes.h"
#include "shared/types/types.h" // for USize, U64, U32

static constexpr auto DYNAMIC_MEMORY_CAPACITY = 1 * MiB;

void allocateSpaceForKernelMemory(Arena scratch, KernelMemory *location);
void convertToKernelMemory(MemoryInfo *memoryInfo, KernelMemory *location);

void findKernelMemory(U64 alignment, U64 numberOfThreads);

U64 alignVirtual(U64 physical, U64 virt, U64 bytes);

U64 mapMemoryWithFlags(U64 virt, U64 physical, U64 bytes, U64 flags);
U64 mapMemory(U64 virt, U64 physical, U64 bytes);

#endif
