#ifndef SHARED_MEMORY_ALLOCATOR_BUDDY_H
#define SHARED_MEMORY_ALLOCATOR_BUDDY_H

#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/node.h"
#include "shared/trees/red-black/basic.h"
#include "shared/types/array-types.h"

static constexpr auto BUDDY_ORDERS_MAX = 54;

typedef struct {
    RedBlackNodeBasic *blocksFree[BUDDY_ORDERS_MAX];
    Exponent blockSizeSmallest;
    Exponent blockSizeLargest;
} Buddy;

void buddyInit(Buddy *buddy, U64 addressSpace);

void buddyStatusAppend(Buddy *buddy);

// Ensure these addresses are at least aligned to the buddy's smallest block
// size size! addressStart up and addressEndExclusive down
[[nodiscard]] bool buddyFreeRegionAdd(Buddy *buddy, U64 addressStart,
                                      U64 addressEndExclusive,
                                      NodeAllocator *nodeAllocator);

#endif
