#ifndef EFI_MEMORY_PHYSICAL_H
#define EFI_MEMORY_PHYSICAL_H

#include "efi/firmware/memory.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black/memory-manager.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h"

typedef struct {
    USize memoryMapSize;
    MemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;

MemoryInfo getMemoryInfo(Arena *perm);

static constexpr auto MAX_KERNEL_STRUCTURES = 32;
extern Memory_max_a kernelStructureLocations;

void initKernelStructureLocations(Arena *perm);

U64 findHighestMemoryAddress(U64 currentHighestAddress, Arena scratch);
U64 allocateKernelStructure(U64 bytes, U64 minimumAlignment,
                            bool tryEncompassingVirtual, Arena scratch);
U64 allocateBytesInUefiPages(U64 bytes, bool isKernelStructure);

RedBlackNodeMM_max_a createArrayForMemoryAllocator(U64 requiredNumberOfNodes,
                                                   Arena scratch);
RedBlackNodeMMPtr_max_a
createFreeListForMemoryAllocator(U64 requiredNumberOfNodes, Arena scratch);

#define FOR_EACH_DESCRIPTOR(memoryInfoAddress, descriptorName)                 \
    for (MemoryDescriptor *descriptorName = (memoryInfoAddress)->memoryMap;    \
         (U64)descriptorName < ((U64)(memoryInfoAddress)->memoryMap +          \
                                (memoryInfoAddress)->memoryMapSize);           \
         descriptorName =                                                      \
             (MemoryDescriptor *)((U8 *)descriptorName +                       \
                                  (memoryInfoAddress)->descriptorSize))

#endif
