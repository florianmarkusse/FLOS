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

// static PageFrame *getBuddy(Buddy *buddy, PageFrame *pageFrame, U8 order) {
//     return (PageFrame *)((U64)pageFrame ^ (getBlockSize(buddy, order)));
// }

void buddyStatusAppend(Buddy *buddy) {
    U32 iterations = buddy->blockSizeLargest - buddy->blockSizeSmallest + 1;
    U64 blockSize = (1 << buddy->blockSizeSmallest);
    for (U32 i = 0; i < iterations; i++, blockSize *= 2) {
        INFO(STRING("order: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(i), 2));
        INFO(STRING(" block size: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(blockSize), 13));
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
    }
}

bool buddyFreeRegionAdd(Buddy *buddy, U64 addressStart, U64 addressEndExclusive,
                        NodeAllocator *nodeAllocator) {
    U64_pow2 blockSizeSmallest = 1 << buddy->blockSizeSmallest;
    ASSERT(addressStart == alignUp(addressStart, blockSizeSmallest));
    ASSERT(addressEndExclusive ==
           alignDown(addressEndExclusive, blockSizeSmallest));

    Exponent maxOrder = buddy->blockSizeLargest - buddy->blockSizeSmallest;
    Exponent bias = maxOrder + ((sizeof(U64) * BITS_PER_BYTE) -
                                (buddy->blockSizeLargest) - 1);
    while (addressStart < addressEndExclusive) {
        Exponent orderToAdd = MIN(
            maxOrder, (Exponent)(bias - (__builtin_clzll(addressEndExclusive -
                                                         addressStart))));

        RedBlackNodeBasic *node = nodeAllocatorGet(nodeAllocator);
        if (!node) {
            return false;
        }
        node->value = addressStart;
        insertRedBlackNodeBasic(&buddy->blocksFree[orderToAdd], node);
        addressStart += getBlockSize(buddy, orderToAdd);
    }

    return true;
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
        INFO(STRING("pageSizesSmallest():\t\t"));
        INFO(buddy->blockSizeSmallest, .flags = NEWLINE);
        INFO(STRING("largestBlockSize:\t\t"));
        INFO(buddy->blockSizeLargest, .flags = NEWLINE);
        INFO(STRING("given space:\t\t\t"));
        INFO(addressSpace, .flags = NEWLINE);
        INFO(STRING("given space:\t\t\t"));
        INFO((void *)addressSpace, .flags = NEWLINE);
    }
}
