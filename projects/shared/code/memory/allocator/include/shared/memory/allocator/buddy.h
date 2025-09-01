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
} Buddy;

Buddy *buddyInit(U64 addressSpace, Arena *perm);

#endif
