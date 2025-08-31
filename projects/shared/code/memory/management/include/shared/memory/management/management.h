#ifndef SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H
#define SHARED_MEMORY_MANAGEMENT_MANAGEMENT_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/memory-manager.h"

extern MMTreeWithFreeList virtualMA;
extern MMTreeWithFreeList physicalMA;

void insertMMNodeAndAddToFreelist(MMTreeWithFreeList *treeWithFreeList,
                                  MMNode *newNode);

void initMemoryManagers(PackedKernelMemory *kernelMemory);

void *allocVirtualMemory(U64 size, U64_pow2 align);
void freeVirtualMemory(Memory memory);

void *allocPhysicalMemory(U64 bytes, U64_pow2 align);
void freePhysicalMemory(Memory memory);

#endif
