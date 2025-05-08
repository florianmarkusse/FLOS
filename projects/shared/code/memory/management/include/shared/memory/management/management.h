#ifndef SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H
#define SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/memory-manager.h"

typedef struct {
    Arena arena;
    RedBlackNodeMMPtr_a freeList;
    RedBlackNodeMM *tree;
} MemoryAllocator;

extern MemoryAllocator virt;
extern MemoryAllocator physical;

void initVirtualMemoryManager(PackedMemoryTree virtualMemoryTree);
void *allocVirtualMemory(U64 size, U64 align);
void freeVirtualMemory(Memory memory);

U64 getVirtualMemoryWithPhysical(U64 size, U64 physical);

void initPhysicalMemoryManager(PackedMemoryTree physicalMemoryTree);
void *allocPhysicalMemory(U64 bytes, U64 align);
void freePhysicalMemory(Memory memory);

#endif
