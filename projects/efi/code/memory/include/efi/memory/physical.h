#ifndef EFI_MEMORY_PHYSICAL_H
#define EFI_MEMORY_PHYSICAL_H

#include "efi/firmware/memory.h"
#include "efi/firmware/system.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

static constexpr U64 UEFI_PAGE_SIZE = 1 << 12;

static constexpr auto MAX_KERNEL_STRUCTURES = 32;

typedef struct {
    Memory buf[MAX_KERNEL_STRUCTURES];
    U32 len;
} KernelStructures;
extern KernelStructures kernelStructureLocations;

typedef struct {
    USize memoryMapSize;
    MemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;

[[nodiscard]] MemoryInfo memoryInfoGet(Arena *perm);

[[nodiscard]] bool memoryTypeCanBeUsedByKernel(MemoryType type);

[[nodiscard]] __attribute__((malloc, alloc_align(2))) void *
alignedMemoryBlockAlloc(U64_pow2 bytes, U64_pow2 alignment, Arena scratch,
                       bool attemptLargestMapping);

[[nodiscard]] __attribute__((malloc, aligned(UEFI_PAGE_SIZE))) void *
pagesAlloc(U64 bytes);

[[nodiscard]] U64 highestMemoryAddressFind(U64 currentHighestAddress,
                                           Arena scratch);

#define FOR_EACH_DESCRIPTOR(memoryInfoAddress, descriptorName)                 \
    for (MemoryDescriptor * (descriptorName) = (memoryInfoAddress)->memoryMap; \
         (U64)(descriptorName) < ((U64)(memoryInfoAddress)->memoryMap +        \
                                  (memoryInfoAddress)->memoryMapSize);         \
         (descriptorName) =                                                    \
             (MemoryDescriptor *)((U8 *)(descriptorName) +                     \
                                  (memoryInfoAddress)->descriptorSize))

#endif
