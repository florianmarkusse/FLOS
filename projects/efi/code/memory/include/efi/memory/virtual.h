#ifndef EFI_MEMORY_VIRTUAL_H
#define EFI_MEMORY_VIRTUAL_H

#include "shared/types/numeric.h"

U64 alignVirtual(U64 virtualAddress, U64 physicalAddress, U64 bytes);

[[nodiscard]] U64 mapMemory(U64 virt, U64 physical, U64 bytes, U64 flags);

#endif
