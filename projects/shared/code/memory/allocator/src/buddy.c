#include "shared/memory/allocator/buddy.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"
#include "shared/memory/management/definitions.h"

static U64_pow2 getBlockSize(Buddy *buddy, U8 order) {
    return ((1 << (buddy->smallestBlockSize + order)));
}

static PageFrame *getBuddy(Buddy *buddy, PageFrame *pageFrame, U8 order) {
    return (PageFrame *)((U64)pageFrame ^ (getBlockSize(buddy, order)));
}

// static void excessMemory(Buddy *buddy, U64 address, U64 size) {
//     ASSERT(isPowerOf2(size));
//     ASSERT(isPowerOf2(size));
// }

static void addToFreeList(Buddy *buddy, U32 order, PageFrame *pageFrame) {
    if (buddy->freePageFrames[order]) {
        buddy->freePageFrames[order]->previous = pageFrame;
    }

    pageFrame->previous = nullptr;
    pageFrame->next = buddy->freePageFrames[order];

    buddy->freePageFrames[order] = pageFrame;
}

void buddyStatusAppend(Buddy *buddy) {
    U32 iterations = buddy->largestBlockSize - buddy->smallestBlockSize + 1;
    U64 blockSize = (1 << buddy->smallestBlockSize);
    for (U32 i = 0; i < iterations; i++, blockSize *= 2) {
        INFO(STRING("order: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(i), 2));
        INFO(STRING(" block size: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(blockSize), 13));
        U64 freePageFrameCount = 0;
        PageFrame *freePageFrame = buddy->freePageFrames[i];
        while (freePageFrame) {
            freePageFrameCount++;
            freePageFrame = freePageFrame->next;
        }
        INFO(STRING("Free: "));
        INFO(freePageFrameCount, .flags = NEWLINE);
    }
}

void buddyFreeRegionAdd(Buddy *buddy, U64 addressStart,
                        U64 addressEndExclusive) {
    Exponent maxOrder = buddy->largestBlockSize - buddy->smallestBlockSize;
    Exponent bias =
        maxOrder + (Exponent)__builtin_clzll(1 << (buddy->largestBlockSize));
    while (addressStart < addressEndExclusive) {
        Exponent orderToAdd = MIN(
            maxOrder, (Exponent)(bias - (__builtin_clzll(addressEndExclusive -
                                                         addressStart))));

        addToFreeList(
            buddy, orderToAdd,
            &buddy->pageFrames[addressStart >> (buddy->smallestBlockSize)]);
        addressStart += getBlockSize(buddy, orderToAdd);
    }
}

Buddy *buddyInit(U64 addressSpace, U64_pow2 largestBlockSize,
                 U64_pow2 smallestBlockSize, Arena *perm) {
    ASSERT(addressSpace >= smallestBlockSize);
    ASSERT(largestBlockSize >= smallestBlockSize);

    largestBlockSize = MIN(floorPowerOf2(addressSpace), largestBlockSize);

    // NOTE: Need to align it up to the next power of 2 of the largest block
    // size. Otherwise we may end up with an odd number of largest blocks, which
    // would be bad.
    U64 addressSpaceAlignedUpLargestPageSize =
        alignUp(addressSpace, largestBlockSize * 2);

    U64 requiredPageFrames = addressSpaceAlignedUpLargestPageSize >>
                             __builtin_ctzll(smallestBlockSize);

    Exponent pageSizeLargestExponents = (U8)__builtin_ctzll(largestBlockSize);
    Exponent pageSizeSmallestExponents = (U8)__builtin_ctzll(smallestBlockSize);

    U8 totalOrders = pageSizeLargestExponents - pageSizeSmallestExponents + 1;

    KFLUSH_AFTER {
        INFO(STRING("required:\t\t\t"));
        INFO(requiredPageFrames, .flags = NEWLINE);
        INFO(STRING("size:\t\t\t"));
        INFO(sizeof(PageFrame), .flags = NEWLINE);
    }

    Buddy *result = NEW(perm, typeof(*result));
    result->smallestBlockSize = pageSizeSmallestExponents;
    result->largestBlockSize = pageSizeLargestExponents;
    result->freePageFrames = NEW(perm, typeof(*result->freePageFrames),
                                 .count = totalOrders, .flags = ZERO_MEMORY);
    result->pageFrames = NEW(perm, typeof(*result->pageFrames),
                             .count = requiredPageFrames, .flags = ZERO_MEMORY);

    U64 addressSpaceAlignedDownSmallestBlockSize =
        alignDown(addressSpace, smallestBlockSize);

    KFLUSH_AFTER {
        INFO(STRING("size of buddy:\t\t\t"));
        INFO(sizeof(Buddy), .flags = NEWLINE);
        INFO(STRING("required page frames:\t\t"));
        INFO(requiredPageFrames, .flags = NEWLINE);
        INFO(STRING("smallestBlockSize:\t\t"));
        INFO(result->smallestBlockSize, .flags = NEWLINE);
        INFO(STRING("largestBlockSize:\t\t"));
        INFO(result->largestBlockSize, .flags = NEWLINE);
        INFO(STRING("total orders:\t\t\t"));
        INFO(totalOrders, .flags = NEWLINE);
        INFO(STRING("given space:\t\t\t"));
        INFO(addressSpace, .flags = NEWLINE);
        INFO(STRING("given space:\t\t\t"));
        INFO((void *)addressSpace, .flags = NEWLINE);
        INFO(STRING("aligned down smallest block:\t"));
        INFO(addressSpaceAlignedDownSmallestBlockSize, .flags = NEWLINE);
        INFO(STRING("aligned down smallest block:\t"));
        INFO((void *)addressSpaceAlignedDownSmallestBlockSize,
             .flags = NEWLINE);
        INFO(STRING("aligned up address space:\t"));
        INFO(addressSpaceAlignedUpLargestPageSize, .flags = NEWLINE);
        INFO(STRING("aligned up address space:\t"));
        INFO((void *)addressSpaceAlignedUpLargestPageSize, .flags = NEWLINE);
    }

    return result;
}
