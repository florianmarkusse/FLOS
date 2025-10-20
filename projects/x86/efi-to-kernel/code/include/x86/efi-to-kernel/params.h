#ifndef X86_EFI_TO_KERNEL_PARAMS_H
#define X86_EFI_TO_KERNEL_PARAMS_H

#include "shared/types/numeric.h"
#include "x86/memory/virtual.h"

// NOTE: Used for crossing ABI boundaries.

typedef struct {
    U64 tscFrequencyPerMicroSecond;
    U8 *XSAVELocation;
    PageMetaDataNode rootPageMetaData;
} X86ArchParams;

#endif
