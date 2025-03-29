#ifndef EFI_MEMORY_PHYSICAL_H
#define EFI_MEMORY_PHYSICAL_H

#include "efi/firmware/memory.h"
#include "shared/memory/allocator/arena.h"
#include "shared/types/array-types.h"
#include "shared/types/types.h"

typedef struct {
    USize memoryMapSize;
    MemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;

MemoryInfo getMemoryInfo(Arena *perm);

extern U64_max_a kernelStructureLocations;

void initKernelStructureLocations(Arena *perm);
void addAddressToKernelStructure(U64 address);

U64 findHighestMemoryAddress(U64 currentHighestAddress, Arena scratch);
U64 allocateKernelStructure(U64 bytes, U64 minimumAlignment,
                            bool tryEncompassingVirtual, Arena scratch);
U64 allocateUnalignedMemory(U64 bytes, bool isKernelStructure);

U64 mappingSize(U64 physical, U64 bytes, bool tryEncompassingVirtual);
U64 alignVirtual(U64 virt, U64 physical, U64 bytes);

#define FOR_EACH_DESCRIPTOR(memoryInfoAddress, descriptorName)                 \
    for (MemoryDescriptor *descriptorName = (memoryInfoAddress)->memoryMap;    \
         (U64)descriptorName < ((U64)(memoryInfoAddress)->memoryMap +          \
                                (memoryInfoAddress)->memoryMapSize);           \
         descriptorName =                                                      \
             (MemoryDescriptor *)((U8 *)descriptorName +                       \
                                  (memoryInfoAddress)->descriptorSize))

#endif
