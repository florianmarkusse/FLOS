#include "shared/memory/converter.h"

#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/types/numeric.h"
static bool isPageSizeValid(U64_pow2 pageSize) {
    ASSERT(!(ringBufferIndex(pageSize, pageSize)));
    return pageSize & pageSizesAvailableMask();
}

U64_pow2 static pageAligned(U64 bytes) {
    return MIN(MAX(ceilingPowerOf2(bytes), pageSizeSmallest()),
               pageSizeLargest());
}

U64_pow2 increasePageSize(U64_pow2 pageSize) {
    U64 largerPages =
        (~((pageSize | (pageSize - 1)))) & pageSizesAvailableMask();

    return largerPages & -largerPages;
}

U64_pow2 decreasePageSize(U64_pow2 pageSize) {
    U64 smallerPages = ((pageSize - 1) & pageSizesAvailableMask());
    return 1ULL << (((sizeof(U64) * BITS_PER_BYTE) - 1) -
                    (U64)(__builtin_clzll(smallerPages)));
}

U64_pow2 pageSizeEncompassing(U64 bytes) {
    U64_pow2 result = pageAligned(bytes);
    if (isPageSizeValid(result)) {
        return result;
    }
    return increasePageSize(result);
}

U64_pow2 pageSizeFitting(U64 bytes) {
    U64_pow2 result = pageAligned(bytes);
    if (isPageSizeValid(result)) {
        return result;
    }
    return decreasePageSize(result);
}

static U64_pow2 largestAlignedPage(U64 address) {
    ASSERT(!(ringBufferIndex(address, pageSizeSmallest())));

    if (address == 0) {
        return pageSizeLargest();
    }

    U64_pow2 result = (1ULL << __builtin_ctzll(address));

    if (isPageSizeValid(result)) {
        return result;
    }

    return decreasePageSize(result);
}

U64_pow2 pageSizeLeastLargerThan(U64 address, U64 bytes) {
    U64_pow2 maxPageSize = largestAlignedPage(address);
    U64_pow2 alignedBytes = pageAligned(bytes);

    if (alignedBytes > maxPageSize) {
        return maxPageSize;
    }

    if (isPageSizeValid(alignedBytes)) {
        return alignedBytes;
    }
    return increasePageSize(alignedBytes);
}
