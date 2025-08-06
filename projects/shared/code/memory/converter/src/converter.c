#include "shared/memory/converter.h"

#include "abstraction/memory/virtual/converter.h"
#include "shared/assert.h"
#include "shared/maths.h"
#include "shared/types/numeric.h"
static bool isPageSizeValid(U64 pageSize) {
    ASSERT(!(RING_RANGE_VALUE(pageSize, pageSize)));
    return pageSize & pageSizesAvailableMask();
}

U64 static pageAligned(U64 bytes) {
    return MIN(MAX(ceilingPowerOf2(bytes), pageSizesSmallest()),
               pageSizesLargest());
}

U64 increasePageSize(U64 pageSize) {
    U64 largerPages =
        (~((pageSize | (pageSize - 1)))) & pageSizesAvailableMask();

    return largerPages & -largerPages;
}

U64 decreasePageSize(U64 pageSize) {
    U64 smallerPages = ((pageSize - 1) & pageSizesAvailableMask());
    return 1ULL << (((sizeof(U64) * BITS_PER_BYTE) - 1) -
                    (U64)(__builtin_clzll(smallerPages)));
}

U64 pageSizeEncompassing(U64 bytes) {
    U64 result = pageAligned(bytes);
    if (isPageSizeValid(result)) {
        return result;
    }
    return increasePageSize(result);
}

U64 pageSizeFitting(U64 bytes) {
    U64 result = pageAligned(bytes);
    if (isPageSizeValid(result)) {
        return result;
    }
    return decreasePageSize(result);
}

static U64 largestAlignedPage(U64 address) {
    ASSERT(!(RING_RANGE_VALUE(address, pageSizesSmallest())));

    if (address == 0) {
        return pageSizesLargest();
    }

    U64 result = (1ULL << __builtin_ctzll(address));

    if (isPageSizeValid(result)) {
        return result;
    }

    return decreasePageSize(result);
}

// NOTE: ready for code generation
// Only difference is multiplication / division
U64 pageSizeLeastLargerThan(U64 address, U64 bytes) {
    U64 maxPageSize = largestAlignedPage(address);
    U64 alignedBytes = pageAligned(bytes);

    if (alignedBytes > maxPageSize) {
        return maxPageSize;
    }

    if (isPageSizeValid(alignedBytes)) {
        return alignedBytes;
    }
    return increasePageSize(alignedBytes);
}
