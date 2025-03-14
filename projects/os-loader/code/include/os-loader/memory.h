#ifndef OS_LOADER_MEMORY_BOOT_FUNCTIONS_H
#define OS_LOADER_MEMORY_BOOT_FUNCTIONS_H

#include "efi-to-kernel/kernel-parameters.h"
#include "efi/firmware/base.h" // for PhysicalAddress
#include "efi/firmware/memory.h"
#include "efi/memory.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h" // for USize, U64, U32

static constexpr auto DYNAMIC_MEMORY_CAPACITY = 64 * UEFI_PAGE_SIZE;
static constexpr auto DYNAMIC_MEMORY_ALIGNMENT = UEFI_PAGE_SIZE;

typedef struct {
    USize memoryMapSize;
    MemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;
MemoryInfo getMemoryInfo();

KernelMemory stubMemoryBeforeExitBootServices(MemoryInfo *memoryInfo);
KernelMemory convertToKernelMemory(MemoryInfo *memoryInfo, KernelMemory result);

void mapWithSmallestNumberOfPagesInKernelMemory(U64 virt, U64 physical,
                                                U64 bytes);
void identityMapPhysicalMemory(U64 currentHighestAddress);

U64 getPhysicalMemory(U64 bytes, U64 alignment);

void findKernelMemory(U64 alignment, U64 numberOfThreads);

#endif
