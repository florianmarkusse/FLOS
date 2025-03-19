#ifndef EFI_MEMORY_PHYSICAL_H
#define EFI_MEMORY_PHYSICAL_H

#include "efi/firmware/memory.h"
#include "shared/memory/allocator/arena.h"
#include "shared/types/types.h"

typedef struct {
    USize memoryMapSize;
    MemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;

MemoryInfo prepareMemoryInfo();
void fillMemoryInfo(MemoryInfo *memoryInfo);

U64 getAlignedPhysicalMemoryWithArena(U64 bytes, U64 alignment, Arena scratch);
U64 getAlignedPhysicalMemory(U64 bytes, U64 alignment);

#define FOR_EACH_DESCRIPTOR(memoryInfoAddress, descriptorName)                 \
    for (MemoryDescriptor *descriptorName = (memoryInfoAddress)->memoryMap;    \
         (U64)descriptorName < ((U64)(memoryInfoAddress)->memoryMap +          \
                                (memoryInfoAddress)->memoryMapSize);           \
         descriptorName =                                                      \
             (MemoryDescriptor *)((U8 *)descriptorName +                       \
                                  (memoryInfoAddress)->descriptorSize))

#endif
