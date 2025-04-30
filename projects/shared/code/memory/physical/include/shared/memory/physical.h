#ifndef SHARED_MEMORY_PHYSICAL_H
#define SHARED_MEMORY_PHYSICAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black-memory-manager.h"
#include "shared/types/numeric.h"

void initPhysicalMemoryManager(PhysicalMemory kernelMemory);
void freeMemory(Memory memory);
void *allocPhysicalMemory(U64 bytes);

extern RedBlackNodeMM *tree;

#endif
