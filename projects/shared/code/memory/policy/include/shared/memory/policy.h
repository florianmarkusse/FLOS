#ifndef SHARED_MEMORY_POLICY_H
#define SHARED_MEMORY_POLICY_H

#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

__attribute__((malloc, alloc_align(1))) void *
allocateIdentityMemory(U64_pow2 blockSize);
void freeIdentityMemory(Memory memory);

__attribute__((malloc, alloc_align(1))) void *
allocateMappableMemory(U64_pow2 blockSize, U64_pow2 mappingSize);
void freeMappableMemory(Memory memory);

#endif
