#ifndef SHARED_MEMORY_VIRTUAL_H
#define SHARED_MEMORY_VIRTUAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/numeric.h"

// NOTE: The free virtual memory regions in sorted order!
extern Range_max_a freeVirtualMemory;

void initVirtualMemoryManager(VirtualMemory virt);
U64 getVirtualMemory(U64 size, U64 align);

#endif
