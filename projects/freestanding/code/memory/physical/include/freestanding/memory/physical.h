#ifndef FREESTANDING_MEMORY_PHYSICAL_H
#define FREESTANDING_MEMORY_PHYSICAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/trees/red-black.h"
#include "shared/types/types.h"

void initPhysicalMemoryManager(PhysicalMemory kernelMemory);
void freeMemory(Memory memory);
void *allocPhysicalMemory(U64 bytes);

extern RedBlackNode *tree;

#endif
