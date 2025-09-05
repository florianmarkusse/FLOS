#ifndef SHARED_MEMORY_ALLOCATOR_BUDDY_H
#define SHARED_MEMORY_ALLOCATOR_BUDDY_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/array-types.h"

typedef struct PageFrame PageFrame;
typedef struct PageFrame {
    struct PageFrame *next;
    struct PageFrame *previous;
    bool used;
} PageFrame;

typedef struct {
    PageFrame **freePageFrames;
    PageFrame *pageFrames;
    Exponent smallestBlockSize;
    Exponent largestBlockSize;
} Buddy;

Buddy *buddyInit(U64 addressSpace, U64_pow2 largestBlockSize,
                 U64_pow2 smallestBlockSize, Arena *perm);

void buddyStatusAppend(Buddy *buddy);

// Ensure these addresses are at least aligned to the buddy's smallest block
// size size! addressStart up and addressEndExclusive down
void buddyFreeRegionAdd(Buddy *buddy, U64 addressStart,
                        U64 addressEndExclusive);

#endif
