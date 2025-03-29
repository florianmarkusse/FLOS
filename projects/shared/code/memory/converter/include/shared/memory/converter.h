#ifndef SHARED_MEMORY_CONVERTER_H
#define SHARED_MEMORY_CONVERTER_H

#include "shared/types/array.h"
#include "shared/types/types.h"

typedef struct {
    U64 numberOfPages;
    U64 pageSize;
} Pages;

typedef ARRAY(Pages) Pages_a;

// TODO: rethink returning Pages from here instead of just a U64 ??
Pages convertPreferredPageToAvailablePages(Pages pages);

// Converts the given bytes to a sensible conversion of available page sizes.
// I.e., If you pass (1 MiB >= x >= 2MiB), it will return 1 page of size 2 MiB.
Pages convertBytesToPagesRoundingUp(U64 bytes);

// Can be used for finding the right mqpping sizes. When addresses have
// differing alignments, the largest page size won't change. However, if you
// start both at (1 GiB - 2 MiB), it will return a page size of 2 MiB on the
// fist call. Of course, afterwards, it might return 1 GiB.

// Will find the most fitting aligned page size, never going beyond bytes.
U64 convertToMostFittingAlignedPageSize(U64 virt, U64 physical, U64 bytes);

// Will find the largest aligned page size, can go beyond the number of bytes.
U64 convertToLargestAlignedPageSize(U64 virt, U64 physical, U64 bytes);

U64 decreasePage(U64 pageSize);

U64 pageEncompassing(U64 bytes);

U64 pageSizeLeastLargerThan(U64 address, U64 bytes);
U64 pageSizeFitting(U64 address, U64 bytes);

#endif
