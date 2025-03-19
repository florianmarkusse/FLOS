#ifndef OS_LOADER_MEMORY_BOOT_FUNCTIONS_H
#define OS_LOADER_MEMORY_BOOT_FUNCTIONS_H

#include "efi-to-kernel/kernel-parameters.h"
#include "efi/firmware/base.h" // for PhysicalAddress
#include "efi/firmware/memory.h"
#include "efi/memory/definitions.h"
#include "efi/memory/physical.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h" // for USize, U64, U32

static constexpr auto DYNAMIC_MEMORY_CAPACITY = 64 * UEFI_PAGE_SIZE;
static constexpr auto DYNAMIC_MEMORY_ALIGNMENT = UEFI_PAGE_SIZE;

KernelMemory stubMemoryBeforeExitBootServices(MemoryInfo *memoryInfo);
KernelMemory convertToKernelMemory(MemoryInfo *memoryInfo, KernelMemory result);

void mapWithSmallestNumberOfPagesInKernelMemory(U64 virt, U64 physical,
                                                U64 bytes);
void identityMapPhysicalMemory(U64 currentHighestAddress);

void findKernelMemory(U64 alignment, U64 numberOfThreads);

#endif
