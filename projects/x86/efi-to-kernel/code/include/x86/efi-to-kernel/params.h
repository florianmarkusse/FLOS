#ifndef X86_EFI_TO_KERNEL_PARAMS_H
#define X86_EFI_TO_KERNEL_PARAMS_H

#include "shared/types/numeric.h"
#include "x86/memory/virtual.h"

// NOTE: Used for crossing ABI boundaries here, so ensuring that both
// targets agree on the size of the struct!
// So, only use this struct for transfering data. Convert this data into native
// structs for processing. Otherwise, performance will suffer.

typedef struct __attribute__((packed)) {
    U16 entriesMapped;
    U16 entriesMappedWithSmallerGranularity;
} PackedPageMetaData;

typedef struct __attribute__((packed)) {
    struct PackedPageMetaDataNode *children;
    PackedPageMetaData metaData;
} PackedPageMetaDataNode;

typedef struct __attribute__((packed)) {
    U64 tscFrequencyPerMicroSecond;
    PackedPageMetaDataNode rootPageMetaData;
    U8 *XSAVELocation;
} X86ArchParams;

#endif
