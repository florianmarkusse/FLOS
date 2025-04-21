#ifndef ABSTRACTION_MEMORY_MANAGEMENT_INIT_H
#define ABSTRACTION_MEMORY_MANAGEMENT_INIT_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/types.h"

void initMemoryManager(KernelMemory memory);
U64 initScreenMemory(U64 physicalScreenAddress, U64 bytes);

#endif
