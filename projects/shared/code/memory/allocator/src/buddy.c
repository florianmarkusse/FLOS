#include "shared/memory/allocator/buddy.h"
#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/management/definitions.h"

Exponent buddyOrderMax(Buddy *buddy) {
    return buddy->data.blockSizeLargest - buddy->data.blockSizeSmallest;
}

Exponent buddyOrderCount(Buddy *buddy) { return buddyOrderMax(buddy) + 1; }

U64_pow2 buddyBlockSize(Buddy *buddy, U8 order) {
    return ((1ULL << (buddy->data.blockSizeSmallest + order)));
}

static U64 getBuddyAddress(U64 address, U64_pow2 blockSize) {
    return (address ^ (blockSize));
}

void *buddyAllocate(Buddy *buddy, U64_pow2 blockSize,
                    NodeAllocator *nodeAllocator) {
    ASSERT(blockSize >= 1ULL << buddy->data.blockSizeSmallest);
    ASSERT(blockSize <= 1ULL << buddy->data.blockSizeLargest);

    Exponent orderRequested =
        (Exponent)__builtin_ctzll(blockSize) - buddy->data.blockSizeSmallest;
    Exponent orderFound = orderRequested;

    RedBlackNodeBasic *pop =
        popRedBlackNodeBasic(&buddy->data.blocksFree[orderFound]);
    while (!pop) {
        orderFound++;
        if (orderFound > buddy->data.blockSizeLargest) {
            longjmp(buddy->jmpBuf, 1);
        }
        blockSize *= 2;
        pop = popRedBlackNodeBasic(&buddy->data.blocksFree[orderFound]);
    }

    while (orderFound > orderRequested) {
        RedBlackNodeBasic *splitNode = nodeAllocatorGet(nodeAllocator);
        if (!splitNode) {
            insertRedBlackNodeBasic(&buddy->data.blocksFree[orderFound], pop);
            longjmp(buddy->jmpBuf, 1);
        }
        orderFound--;
        blockSize /= 2;

        splitNode->value = getBuddyAddress(pop->value, blockSize);
        insertRedBlackNodeBasic(&buddy->data.blocksFree[orderFound], splitNode);
    }

    return (void *)pop->value;
}

void buddyFree(Buddy *buddy, Memory memory, NodeAllocator *nodeAllocator) {
    ASSERT(memory.start ==
           alignUp(memory.start, 1 << buddy->data.blockSizeSmallest));
    ASSERT(isAlignedTo(memory.bytes, 1 << buddy->data.blockSizeSmallest));

    Exponent maxOrder = buddyOrderMax(buddy);
    Exponent bias = maxOrder + ((sizeof(U64) * BITS_PER_BYTE) -
                                (buddy->data.blockSizeLargest) - 1);

    while (memory.bytes) {
        // block size given the size of the region to add
        Exponent orderToAdd =
            MIN(maxOrder, (Exponent)(bias - (__builtin_clzll(memory.bytes))));

        // block size given the alignment constraints
        if (memory.start) {
            orderToAdd =
                MIN(orderToAdd, (Exponent)__builtin_ctzll(memory.start) -
                                    buddy->data.blockSizeSmallest);
        }

        RedBlackNodeBasic *node = nodeAllocatorGet(nodeAllocator);
        if (!node) {
            longjmp(buddy->jmpBuf, 1);
        }

        node->value = memory.start;
        insertRedBlackNodeBasic(&buddy->data.blocksFree[orderToAdd], node);

        U64_pow2 blockSize = buddyBlockSize(buddy, orderToAdd);
        ASSERT(isAlignedTo(node->value, blockSize));
        memory.start += blockSize;
        memory.bytes -= blockSize;
    }
}

void buddyInit(Buddy *buddy, Exponent blockSizeLargest) {
    Exponent blockSizeLargestExponents = blockSizeLargest;
    Exponent blockSizeSmallestExponents =
        (U8)__builtin_ctzll(pageSizesSmallest());

    *buddy = (Buddy){
        .data = (BuddyData){.blockSizeSmallest = blockSizeSmallestExponents,
                            .blockSizeLargest = blockSizeLargestExponents}};
}
