#ifndef SHARED_MEMORY_VIRTUAL_H
#define SHARED_MEMORY_VIRTUAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/types.h"

// TODO: Make this an array of regions instead of hardcoded 2 regions.
extern Memory higherHalfRegion;
extern Memory lowerHalfRegion; // Start is set in the init function.

void initVirtualMemoryManager(VirtualMemory virt);
U64 getVirtualMemory(U64 size, U64 align);

#endif
