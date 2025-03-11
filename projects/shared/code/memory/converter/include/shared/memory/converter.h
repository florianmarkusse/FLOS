#ifndef SHARED_MEMORY_CONVERTER_H
#define SHARED_MEMORY_CONVERTER_H

#include "shared/types/array.h"
#include "shared/types/types.h"

typedef struct {
    U64 numberOfPages;
    U64 pageSize;
} Pages;

typedef ARRAY(Pages) Pages_a;

Pages convertPreferredPageToAvailablePages(Pages pages);

// Converts the given bytes to a sensible conversion of available page sizes.
// I.e., If you pass (2 MiB - 1 KiB), it will return 1 page of size 2 MiB.
Pages convertBytesToPagesRoundingUp(U64 bytesPowerOfTwo);

bool isValidPageSizeForArch(U64 pageSize);
#endif
