#ifndef SHARED_MEMORY_VIRTUAL_H
#define SHARED_MEMORY_VIRTUAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/numeric.h"

extern RedBlackNodeMM *virtualTree;

void initVirtualMemoryManager(MemoryTree virtualMemoryTree);
U64 getVirtualMemory(U64 size, U64 align);

U64 getVirtualMemoryWithPhysical(U64 size, U64 physical);

#endif
