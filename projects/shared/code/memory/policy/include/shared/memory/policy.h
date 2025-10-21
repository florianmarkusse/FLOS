#ifndef SHARED_MEMORY_POLICY_H
#define SHARED_MEMORY_POLICY_H

#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

[[nodiscard]] __attribute__((malloc, alloc_align(1))) void *
identityMemoryAlloc(U64_pow2 blockSize);
void identityMemoryFree(Memory memory);
// NOTE: Losing some memory here to the void, but okay, we are using a buddy
// allocator with minimum size.
void identityMemoryNotBlockSizeFree(Memory memory);

[[nodiscard]] __attribute__((malloc, alloc_align(1))) void *
mappableMemoryAlloc(U64_pow2 blockSize, U64_pow2 mappingSize);
void mappableMemoryFree(Memory memory);

#endif
