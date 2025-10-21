#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "shared/types/numeric.h"
#include "x86/memory/definitions.h"
typedef struct {
    U16 entriesMapped;
    U16 entriesMappedWithSmallerGranularity;
} PageMetaData;

typedef struct PageMetaDataNode {
    struct PageMetaDataNode *children;
    PageMetaData metaData;
} PageMetaDataNode;

extern VirtualPageTable *pageTableRoot;
extern PageMetaDataNode pageMetaDataRoot;

[[nodiscard]] VirtualPageTable *pageTableZeroedGet();

#endif
