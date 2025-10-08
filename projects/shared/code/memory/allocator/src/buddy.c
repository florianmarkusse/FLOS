#include "shared/memory/allocator/buddy.h"
#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/management/definitions.h"

static Exponent smallestPageSizeExponent() {
    return (Exponent)__builtin_ctzll(pageSizesSmallest());
}

static Exponent buddyOrderMaxOnDifference(Exponent blockSizeLargest,
                                          Exponent blockSizeSmallest) {
    return blockSizeLargest - blockSizeSmallest;
}

Exponent buddyOrderMax(Buddy *buddy) {
    return buddyOrderMaxOnDifference(buddy->data.blockSizeLargest,
                                     buddy->data.blockSizeSmallest);
}

Exponent buddyOrderCountOnLargestPageSize(Exponent blockSizeLargest) {
    return buddyOrderMaxOnDifference(blockSizeLargest,
                                     smallestPageSizeExponent()) +
           1;
}

Exponent buddyOrderCount(Buddy *buddy) { return buddyOrderMax(buddy) + 1; }

U64_pow2 buddyBlockSize(Buddy *buddy, U8 order) {
    return ((1ULL << (buddy->data.blockSizeSmallest + order)));
}

static bool buddyBlockRemoveTry(Exponent order, U64 buddyAddress,
                                BuddyData *buddy) {
    U64 *blockBuf = buddy->blocks[order].buf;
    U32 *blockLen = &buddy->blocks[order].len;

    for (typeof(*blockLen) i = 0; i < *blockLen; i++) {
        if (blockBuf[i] == buddyAddress) {
            blockBuf[i] = blockBuf[*blockLen - 1];
            (*blockLen)--;
            return true;
        }
    }

    return false;
}

static U64 getBuddyAddress(U64 address, U64_pow2 blockSize) {
    return (address ^ (blockSize));
}

void *buddyAllocate(Buddy *buddy, U64_pow2 blockSize) {
    ASSERT(blockSize >= 1ULL << buddy->data.blockSizeSmallest);
    ASSERT(blockSize <= 1ULL << buddy->data.blockSizeLargest);

    Exponent orderRequested =
        (Exponent)__builtin_ctzll(blockSize) - buddy->data.blockSizeSmallest;
    Exponent orderFound = orderRequested;

    Exponent orderMax = buddyOrderMax(buddy);
    while (!buddy->data.blocks[orderFound].len) {
        if (orderFound == orderMax) {
            longjmp(buddy->memoryExhausted, 1);
        }

        orderFound++;
        blockSize *= 2;
    }

    U64_a *blocks = &buddy->data.blocks[orderFound];
    U64 address = blocks->buf[blocks->len - 1];
    blocks->len--;

    while (orderFound > orderRequested) {
        orderFound--;
        blocks = &buddy->data.blocks[orderFound];
        blockSize /= 2;

        if (blocks->len == buddy->data.blocksCapacityPerOrder) {
            longjmp(buddy->backingBufferExhausted, 1);
        }
        blocks->buf[blocks->len] = getBuddyAddress(address, blockSize);
        blocks->len++;
    }

    return (void *)address;
}

void buddyFree(Buddy *buddy, Memory memory) {
    ASSERT(memory.start ==
           alignUp(memory.start, 1 << buddy->data.blockSizeSmallest));
    ASSERT(isAlignedTo(memory.bytes, 1 << buddy->data.blockSizeSmallest));

    Exponent maxOrder = buddyOrderMax(buddy);
    Exponent bias = maxOrder + ((sizeof(U64) * BITS_PER_BYTE) -
                                (buddy->data.blockSizeLargest) - 1);

    U64 memoryAddress = memory.start;
    U64 memoryEnd = memory.start + memory.bytes;

    while (memoryAddress < memoryEnd) {
        // block size given the size of the region to add
        Exponent orderToAdd = MIN(
            maxOrder,
            (Exponent)(bias - (__builtin_clzll(memoryEnd - memoryAddress))));

        // block size given the alignment constraints
        if (memoryAddress) {
            orderToAdd =
                MIN(orderToAdd, (Exponent)__builtin_ctzll(memoryAddress) -
                                    buddy->data.blockSizeSmallest);
        }

        U64_pow2 blockSize = buddyBlockSize(buddy, orderToAdd);
        U64 buddyAddress = getBuddyAddress(memoryAddress, blockSize);
        Exponent orderMax = buddyOrderMax(buddy);

        while (orderToAdd < orderMax &&
               buddyBlockRemoveTry(orderToAdd, buddyAddress, &buddy->data)) {
            // Turn off the order's bit, so we always have the "lowest"
            // address buddy, so we can move up an order
            memoryAddress &= (~blockSize);

            blockSize *= 2;
            orderToAdd++;
            buddyAddress = getBuddyAddress(memoryAddress, blockSize);
        }

        U32 *blockLen = &buddy->data.blocks[orderToAdd].len;
        if (*blockLen == buddy->data.blocksCapacityPerOrder) {
            longjmp(buddy->backingBufferExhausted, 1);
        }
        U64 *blockBuf = buddy->data.blocks[orderToAdd].buf;

        blockBuf[*blockLen] = memoryAddress;
        (*blockLen)++;

        memoryAddress += blockSize;
    }
}

void buddyInit(Buddy *buddy, U64 *backingBuffer, U32 blocksCapacity,
               Exponent orderCount) {
    ASSERT(orderCount <= BUDDY_ORDERS_MAX);

    buddy->data.blockSizeSmallest = smallestPageSizeExponent();
    buddy->data.blockSizeLargest =
        buddy->data.blockSizeSmallest + (orderCount - 1);

    for (typeof(orderCount) i = 0; i < orderCount; i++) {
        buddy->data.blocks[i].buf = backingBuffer + (i * blocksCapacity);
        buddy->data.blocks[i].len = 0;
    }

    buddy->data.blocksCapacityPerOrder = blocksCapacity;
}
