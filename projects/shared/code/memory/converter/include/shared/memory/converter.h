#ifndef SHARED_MEMORY_CONVERTER_H
#define SHARED_MEMORY_CONVERTER_H

#include "shared/types/array.h"
#include "shared/types/types.h"

typedef struct {
    U64 numberOfPages;
    U64 pageSize;
} Pages;

typedef ARRAY(Pages) Pages_a;

U64 getLargerPageSizesThan(U64 pageSize);
Pages convertPreferredPageToAvailablePages(Pages pages);

// Converts the given bytes to a sensible conversion of available page sizes.
// I.e., If you pass (1 MiB >= x >= 2MiB), it will return 1 page of size 2 MiB.
Pages convertBytesToPagesRoundingUp(U64 bytes);
Pages convertBytesToSmallestNuberOfPages(U64 bytes);

bool isValidPageSizeForArch(U64 pageSize);
#endif
