#ifndef SHARED_MEMORY_POLICY_H
#define SHARED_MEMORY_POLICY_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

void initMemoryManager(PackedKernelMemory memory);

void *allocateIdentityMemory(U64 bytes);
void freeIdentityMemory(Memory memory);

void *allocateMappableMemory(U64 bytes, U64 align);
void freeMappableMemory(Memory memory);

#endif
