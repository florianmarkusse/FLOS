#ifndef SHARED_MEMORY_POLICY_H
#define SHARED_MEMORY_POLICY_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

void initMemoryManager(KernelMemory memory);

void *allocAndMap(U64 bytes);
void *allocContiguousAndMap(U64 bytes);
void freeMapped(Memory memory);

#endif
