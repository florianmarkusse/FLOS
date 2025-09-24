#ifndef X86_EFI_TO_KERNEL_PARAMS_H
#define X86_EFI_TO_KERNEL_PARAMS_H

#include "shared/types/numeric.h"
#include "x86/memory/virtual.h"

// NOTE: Used for crossing ABI boundaries here, so ensuring that both
// targets agree on the size of the struct!

typedef struct {
    U16 entriesMapped;
    U16 entriesMappedWithSmallerGranularity;
} PackedPageMetaData;

typedef struct {
    struct PackedPageMetaDataNode *children;
    PackedPageMetaData metaData;
} PackedPageMetaDataNode;

typedef struct {
    U64 tscFrequencyPerMicroSecond;
    U8 *XSAVELocation;
    PackedPageMetaDataNode rootPageMetaData;
} X86ArchParams;

#endif
