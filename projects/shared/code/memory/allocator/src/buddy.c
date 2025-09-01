#include "shared/memory/allocator/buddy.h"
#include "abstraction/log.h"
#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"

static void excessMemory(Buddy *buddy, U64 address, U64 size) {
    ASSERT(isPowerOf2(size));
    ASSERT(isPowerOf2(size));
}

static void addToFreeList(Buddy *buddy, U32 order, PageFrame *pageFrame) {
    if (buddy->freePageFrames[order]) {
        buddy->freePageFrames[order]->previous = pageFrame;
    }

    pageFrame->previous = nullptr;
    pageFrame->next = buddy->freePageFrames[order];

    buddy->freePageFrames[order] = pageFrame;
}

Buddy *buddyInit(U64 addressSpace, Arena *perm) {
    U64 addressSpaceAlignedUpLargestPageSize =
        alignUp(addressSpace, pageSizesLargest());

    U64 requiredPageFrames =
        addressSpaceAlignedUpLargestPageSize / pageSizesSmallest();

    Exponent pageSizeLargestExponents = (U8)__builtin_ctzll(pageSizesLargest());
    Exponent pageSizeSmallestExponents =
        (U8)__builtin_ctzll(pageSizesSmallest());

    U8 requiredFreeLists =
        pageSizeLargestExponents - pageSizeSmallestExponents + 1;

    Buddy *result = NEW(perm, typeof(*result));
    result->smallestBlockSize = (Exponent)__builtin_ctzll(pageSizesSmallest());
    result->freePageFrames =
        NEW(perm, typeof(*result->freePageFrames), .count = requiredFreeLists,
            .flags = ZERO_MEMORY);
    result->pageFrames = NEW(perm, typeof(*result->pageFrames),
                             .count = requiredFreeLists, .flags = ZERO_MEMORY);

    U64 pageSizeLargestOffset = pageSizesLargest() / pageSizesSmallest();
    U32 blocksFreeInitial =
        (U32)(addressSpaceAlignedUpLargestPageSize / pageSizesLargest());
    for (U32 i = 0; i < blocksFreeInitial; i++) {
        addToFreeList(result, requiredFreeLists,
                      &result->pageFrames[i * pageSizeLargestOffset]);
    }

    U64 addressSpaceAlignedDownSmallestPageSize =
        alignDown(addressSpace, pageSizesSmallest());

    U64 excessAddressSpace = addressSpaceAlignedUpLargestPageSize -
                             addressSpaceAlignedDownSmallestPageSize;

    KFLUSH_AFTER {
        INFO(STRING("size of buddy: "));
        INFO(sizeof(Buddy), .flags = NEWLINE);
        INFO(STRING("required page frames: "));
        INFO(requiredPageFrames, .flags = NEWLINE);
        INFO(STRING("smallestBlockSize: "));
        INFO(result->smallestBlockSize, .flags = NEWLINE);
        INFO(STRING("required freelists: "));
        INFO(requiredFreeLists, .flags = NEWLINE);
        INFO(STRING("initial free blocks: "));
        INFO(blocksFreeInitial, .flags = NEWLINE);
        INFO(STRING("excess address space: "));
        INFO(excessAddressSpace, .flags = NEWLINE);
    }

    return result;
}
