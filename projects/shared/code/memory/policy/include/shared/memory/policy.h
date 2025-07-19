#ifndef SHARED_MEMORY_POLICY_H
#define SHARED_MEMORY_POLICY_H

#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

void *allocateIdentityMemory(U64 bytes, U64 align);
void freeIdentityMemory(Memory memory);

// align should be a power of 2. And bytes should be a multiple of align.
// NOTE: pageSize to map for future extension should be at most the value of
// align or smaller.
void *allocateMappableMemory(U64 bytes, U64 align);
void freeMappableMemory(Memory memory);

#endif
