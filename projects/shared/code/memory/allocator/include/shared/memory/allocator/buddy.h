#ifndef SHARED_MEMORY_ALLOCATOR_BUDDY_H
#define SHARED_MEMORY_ALLOCATOR_BUDDY_H

#include "shared/types/numeric.h"

typedef struct {
    U64 size;
    bool isFree;
} BuddyBlock;

static inline BuddyBlock *nextBuddy(BuddyBlock *block) {
    return (BuddyBlock *)((I8 *)block + block->size);
}

BuddyBlock *splitBuddy(BuddyBlock *block, U64 size);

BuddyBlock *findBestBuddy(BuddyBlock *head, BuddyBlock *tail, U64 size);

typedef struct {
    BuddyBlock *head;
    BuddyBlock *tail;

    void **jmp_buf;
} BuddyAllocator;

BuddyAllocator createBuddyAllocator(I8 *data, U64 size);

void coalesceBuddies(BuddyBlock *head, BuddyBlock *tail);

__attribute__((malloc)) void *buddyAlloc(BuddyAllocator *buddyAllocator,
                                         I64 size, I64 count, U8 flags);

void freeBuddy(BuddyAllocator *buddyAllocator, void *data);

#endif
