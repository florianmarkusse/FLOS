#ifndef SHARED_MEMORY_ALLOCATOR_BUDDY_H
#define SHARED_MEMORY_ALLOCATOR_BUDDY_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/node.h"
#include "shared/trees/red-black/basic.h"
#include "shared/types/array-types.h"

// TODO: Can update the interface here to add flags to the allocator.
// NULLPTR_ON_FAIL etc. if need be.

static constexpr auto BUDDY_ORDERS_MAX = 52; // in general, 4096 up until 2^63

typedef struct {
    U64_a blocks[BUDDY_ORDERS_MAX];
    U32 blocksCapacityPerOrder;
    Exponent blockSizeSmallest;
    Exponent blockSizeLargest;
} BuddyData;

typedef struct {
    JumpBuffer memoryExhausted;
    JumpBuffer backingBufferExhausted;
    BuddyData data;
} Buddy;

[[nodiscard]] Exponent
buddyOrderCountOnLargestPageSize(Exponent blockSizeLargest);
void buddyInit(Buddy *buddy, U64 *backingBuffer, U32 blocksCapacity,
               Exponent orderCount);

[[nodiscard]] Exponent buddyOrderMax(Buddy *buddy);
[[nodiscard]] Exponent buddyOrderCount(Buddy *buddy);

[[nodiscard]] U64_pow2 buddyBlockSize(Buddy *buddy, U8 order);

[[nodiscard]] __attribute__((malloc, alloc_align(2))) void *
buddyAllocate(Buddy *buddy, U64_pow2 blockSize);

// Ensure these addresses are at least aligned to the buddy's smallest block
// size size! addressStart up and addressEndExclusive down
void buddyFree(Buddy *buddy, Memory memory);

#endif
