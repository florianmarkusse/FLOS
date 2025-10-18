#ifndef EFI_MEMORY_VIRTUAL_H
#define EFI_MEMORY_VIRTUAL_H

#include "shared/types/numeric.h"

[[nodiscard]] U64 alignVirtual(U64 virtualAddress, U64 physicalAddress,
                               U64 bytes);

[[nodiscard]] U64 mapMemory(U64 virt, U64 physical, U64 bytes, U64 flags);

typedef struct {
    U64 stackVirtualTop;
    U64 virtualMemoryFirstAvailable;
} StackResult;
[[nodiscard]] StackResult stackCreateAndMap(U64 virtualMemoryFirstAvailable,
                                            U64 stackSize,
                                            bool attemptLargestMapping);

#endif
