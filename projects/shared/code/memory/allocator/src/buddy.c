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
            longjmp(buddy->memoryExhausted, 1);
        }
        blockSize *= 2;
        pop = popRedBlackNodeBasic(&buddy->data.blocksFree[orderFound]);
    }

    while (orderFound > orderRequested) {
        RedBlackNodeBasic *splitNode = nodeAllocatorGet(nodeAllocator);
        if (!splitNode) {
            insertRedBlackNodeBasic(&buddy->data.blocksFree[orderFound], pop);
            longjmp(buddy->backingBufferExhausted, 1);
        }
        orderFound--;
        blockSize /= 2;

        splitNode->value = getBuddyAddress(pop->value, blockSize);
        insertRedBlackNodeBasic(&buddy->data.blocksFree[orderFound], splitNode);
    }

    void *result = (void *)pop->value;

    nodeAllocatorFree(nodeAllocator, pop);

    return result;
}

void buddyFree(Buddy *buddy, Memory memory, NodeAllocator *nodeAllocator) {
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
        RedBlackNodeBasic *nodeFree = deleteRedBlackNodeBasic(
            &buddy->data.blocksFree[orderToAdd], buddyAddress);
        if (!nodeFree) {
            nodeFree = nodeAllocatorGet(nodeAllocator);
            if (!nodeFree) {
                longjmp(buddy->backingBufferExhausted, 1);
            }
        } else {
            while (1) {
                // Turn off the order's bit, so we always have the "lowest"
                // address buddy, so we can move up an order
                memoryAddress &= (~blockSize);
                blockSize *= 2;
                orderToAdd++;
                buddyAddress = getBuddyAddress(memoryAddress, blockSize);

                RedBlackNodeBasic *nodeHigherOrder = deleteRedBlackNodeBasic(
                    &buddy->data.blocksFree[orderToAdd], buddyAddress);
                if (nodeHigherOrder) {
                    nodeAllocatorFree(nodeAllocator, nodeHigherOrder);
                } else {
                    break;
                }
            }
        }

        nodeFree->value = memoryAddress;
        insertRedBlackNodeBasic(&buddy->data.blocksFree[orderToAdd], nodeFree);

        memoryAddress += blockSize;
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
