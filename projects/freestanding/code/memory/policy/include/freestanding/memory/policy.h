#ifndef FREESTANDING_MEMORY_POLICY_H
#define FREESTANDING_MEMORY_POLICY_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"

void initMemoryManager(KernelMemory memory);

void *allocAndMap(U64 bytes);
void *allocContiguousAndMap(U64 bytes);
void freeMapped(Memory memory);
U64 getVirtualMemory(U64 size, U64 align);

#endif
