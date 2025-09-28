#include "shared/memory/allocator/buddy.h"
#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/management/definitions.h"

U64_pow2 buddyBlockSize(Buddy *buddy, U8 order) {
    return ((1ULL << (buddy->data.blockSizeSmallest + order)));
}

static U64 getBuddyAddress(U64 address, U64_pow2 blockSize) {
    return (address ^ (blockSize));
}

void buddyFree(Buddy *buddy, void *address, U64_pow2 blockSize,
               NodeAllocator *nodeAllocator) {
    ASSERT(blockSize >= 1ULL << buddy->data.blockSizeSmallest);
    ASSERT(blockSize <= 1ULL << buddy->data.blockSizeLargest);

    Exponent order =
        (Exponent)__builtin_ctzll(blockSize) - buddy->data.blockSizeSmallest;
    U64 memoryAddress = (U64)address;

    U64 buddyAddress = getBuddyAddress(memoryAddress, blockSize);
    RedBlackNodeBasic *nodeFree =
        deleteRedBlackNodeBasic(&buddy->data.blocksFree[order], buddyAddress);
    if (!nodeFree) {
        nodeFree = nodeAllocatorGet(nodeAllocator);
        if (!nodeFree) {
            longjmp(buddy->jmpBuf, 1);
        }
    } else {
        while (1) {
            // Turn off the order's bit, so we always have the "lowest" address
            // buddy, so we can move up an order
            memoryAddress &= (~blockSize);
            blockSize *= 2;
            order++;
            buddyAddress = getBuddyAddress(memoryAddress, blockSize);

            RedBlackNodeBasic *nodeHigherOrder = deleteRedBlackNodeBasic(
                &buddy->data.blocksFree[order], buddyAddress);
            if (nodeHigherOrder) {
                nodeAllocatorFree(nodeAllocator, nodeFree);
                nodeFree = nodeHigherOrder;
            } else {
                break;
            }
        }
    }

    nodeFree->value = memoryAddress;
    insertRedBlackNodeBasic(&buddy->data.blocksFree[order], nodeFree);
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

void buddyFreeRegionAdd(Buddy *buddy, U64 addressStart, U64 addressEndExclusive,
                        NodeAllocator *nodeAllocator) {
    ASSERT(addressStart ==
           alignUp(addressStart, 1 << buddy->data.blockSizeSmallest));
    ASSERT(addressEndExclusive ==
           alignDown(addressEndExclusive, 1 << buddy->data.blockSizeSmallest));

    Exponent maxOrder =
        buddy->data.blockSizeLargest - buddy->data.blockSizeSmallest;
    Exponent bias = maxOrder + ((sizeof(U64) * BITS_PER_BYTE) -
                                (buddy->data.blockSizeLargest) - 1);

    U64 remaining = addressEndExclusive - addressStart;
    while (remaining) {
        // block size given the size of the region to add
        Exponent orderToAdd =
            MIN(maxOrder, (Exponent)(bias - (__builtin_clzll(remaining))));

        // block size given the alignment constraints
        if (addressStart) {
            orderToAdd =
                MIN(orderToAdd, (Exponent)__builtin_ctzll(addressStart) -
                                    buddy->data.blockSizeSmallest);
        }

        RedBlackNodeBasic *node = nodeAllocatorGet(nodeAllocator);
        if (!node) {
            longjmp(buddy->jmpBuf, 1);
        }

        node->value = addressStart;
        insertRedBlackNodeBasic(&buddy->data.blocksFree[orderToAdd], node);

        U64_pow2 blockSize = buddyBlockSize(buddy, orderToAdd);
        ASSERT(isAlignedTo(node->value, blockSize));
        addressStart += blockSize;
        remaining -= blockSize;
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
