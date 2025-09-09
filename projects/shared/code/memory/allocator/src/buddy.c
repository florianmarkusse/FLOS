#include "shared/memory/allocator/buddy.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/allocator/node.h"
#include "shared/memory/management/definitions.h"

static U64_pow2 getBlockSize(Buddy *buddy, U8 order) {
    return ((1 << (buddy->blockSizeSmallest + order)));
}

static U64 getBuddyAddress(U64 address, U64_pow2 blockSize) {
    return (address ^ (blockSize));
}

void buddyFree(Buddy *buddy, void *address, U64_pow2 blockSize,
               NodeAllocator *nodeAllocator) {
    ASSERT(blockSize >= 1ULL << buddy->blockSizeSmallest);
    ASSERT(blockSize <= 1ULL << buddy->blockSizeLargest);

    Exponent order =
        (Exponent)__builtin_ctzll(blockSize) - buddy->blockSizeSmallest;
    U64 memoryAddress = (U64)address;

    U64 buddyAddress = getBuddyAddress(memoryAddress, blockSize);
    RedBlackNodeBasic *nodeFree =
        deleteRedBlackNodeBasic(&buddy->blocksFree[order], buddyAddress);
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
                &buddy->blocksFree[order], buddyAddress);
            if (nodeHigherOrder) {
                nodeAllocatorFree(nodeAllocator, nodeFree);
                nodeFree = nodeHigherOrder;
            } else {
                break;
            }
        }
    }

    nodeFree->value = memoryAddress;
    insertRedBlackNodeBasic(&buddy->blocksFree[order], nodeFree);
}

void *buddyAllocate(Buddy *buddy, U64_pow2 blockSize,
                    NodeAllocator *nodeAllocator) {
    ASSERT(blockSize >= 1ULL << buddy->blockSizeSmallest);
    ASSERT(blockSize <= 1ULL << buddy->blockSizeLargest);

    Exponent orderRequested =
        (Exponent)__builtin_ctzll(blockSize) - buddy->blockSizeSmallest;
    Exponent orderFound = orderRequested;

    RedBlackNodeBasic *pop =
        popRedBlackNodeBasic(&buddy->blocksFree[orderFound]);
    while (!pop) {
        orderFound++;
        if (orderFound > buddy->blockSizeLargest) {
            longjmp(buddy->jmpBuf, 1);
        }
        blockSize *= 2;
        pop = popRedBlackNodeBasic(&buddy->blocksFree[orderFound]);
    }

    while (orderFound > orderRequested) {
        RedBlackNodeBasic *splitNode = nodeAllocatorGet(nodeAllocator);
        if (!splitNode) {
            insertRedBlackNodeBasic(&buddy->blocksFree[orderFound], pop);
            longjmp(buddy->jmpBuf, 1);
        }
        orderFound--;
        blockSize /= 2;

        splitNode->value = getBuddyAddress(pop->value, blockSize);
        insertRedBlackNodeBasic(&buddy->blocksFree[orderFound], splitNode);
    }

    return (void *)pop->value;
}

void buddyNodesValueAppend(RedBlackNodeBasic *node) {
    if (!node) {
        return;
    }

    INFO(STRING("\t"));
    INFO((void *)node->value);

    buddyNodesValueAppend(node->children[RB_TREE_LEFT]);
    buddyNodesValueAppend(node->children[RB_TREE_RIGHT]);
}

void buddyStatusAppend(Buddy *buddy) {
    U32 iterations = buddy->blockSizeLargest - buddy->blockSizeSmallest + 1;
    U64 blockSize = (1 << buddy->blockSizeSmallest);
    for (U32 i = 0; i < iterations; i++, blockSize *= 2) {
        INFO(STRING("order: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(i), 2));
        INFO(STRING(" block size: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(blockSize), 13));
        INFO(
            stringWithMinSizeDefault(CONVERT_TO_STRING((void *)blockSize), 20));
        U64 nodesFreeCount = 0;

        RedBlackNodeBasic *tree = buddy->blocksFree[i];
        if (tree) {
            RedBlackNodeBasic *buffer[2 * RB_TREE_MAX_HEIGHT];
            buffer[0] = tree;
            U32 len = 1;

            while (len > 0) {
                RedBlackNodeBasic *node = buffer[len - 1];
                len--;
                nodesFreeCount++;

                for (RedBlackDirection dir = 0; dir < RB_TREE_CHILD_COUNT;
                     dir++) {
                    if (node->children[dir]) {
                        buffer[len] = node->children[dir];
                        len++;
                    }
                }
            }
        }

        INFO(STRING("Free: "));
        INFO(nodesFreeCount, .flags = NEWLINE);

        if (tree) {
            buddyNodesValueAppend(tree);
            INFO(STRING("\n"));
        }
    }
}

void buddyFreeRegionAdd(Buddy *buddy, U64 addressStart, U64 addressEndExclusive,
                        NodeAllocator *nodeAllocator) {
    U64_pow2 blockSizeSmallest = 1 << buddy->blockSizeSmallest;
    ASSERT(addressStart == alignUp(addressStart, blockSizeSmallest));
    ASSERT(addressEndExclusive ==
           alignDown(addressEndExclusive, blockSizeSmallest));

    Exponent maxOrder = buddy->blockSizeLargest - buddy->blockSizeSmallest;
    Exponent bias = maxOrder + ((sizeof(U64) * BITS_PER_BYTE) -
                                (buddy->blockSizeLargest) - 1);

    U64 remaining = addressEndExclusive - addressStart;
    while (remaining) {
        // block size given the size of the region to add
        Exponent orderToAdd =
            MIN(maxOrder, (Exponent)(bias - (__builtin_clzll(remaining))));

        // block size given the alignment constraints
        if (addressStart) {
            orderToAdd =
                MIN(orderToAdd, (Exponent)__builtin_ctzll(addressStart) -
                                    buddy->blockSizeSmallest);
        }

        RedBlackNodeBasic *node = nodeAllocatorGet(nodeAllocator);
        if (!node) {
            longjmp(buddy->jmpBuf, 1);
        }

        node->value = addressStart;
        insertRedBlackNodeBasic(&buddy->blocksFree[orderToAdd], node);

        U64_pow2 blockSize = getBlockSize(buddy, orderToAdd);
        ASSERT(isAlignedTo(node->value, blockSize));
        addressStart += blockSize;
        remaining -= blockSize;
    }
}

void buddyInit(Buddy *buddy, U64 addressSpace) {
    U64 largestBlockSize = floorPowerOf2(addressSpace);
    Exponent blockSizeLargestExponents = (U8)__builtin_ctzll(largestBlockSize);
    Exponent blockSizeSmallestExponents =
        (U8)__builtin_ctzll(pageSizesSmallest());

    *buddy = (Buddy){.blocksFree = {0},
                     .blockSizeSmallest = blockSizeSmallestExponents,
                     .blockSizeLargest = blockSizeLargestExponents};

    KFLUSH_AFTER {
        INFO(STRING("size of buddy:\t\t\t"));
        INFO(sizeof(Buddy), .flags = NEWLINE);
        INFO(STRING("block size smallest:\t\t"));
        INFO(buddy->blockSizeSmallest, .flags = NEWLINE);
        INFO(STRING("block size largest:\t\t"));
        INFO(buddy->blockSizeLargest, .flags = NEWLINE);
        INFO(STRING("given space:\t\t\t"));
        INFO(addressSpace, .flags = NEWLINE);
        INFO(STRING("given space:\t\t\t"));
        INFO((void *)addressSpace, .flags = NEWLINE);
    }
}
