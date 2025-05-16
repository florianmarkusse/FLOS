#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "x86/memory/definitions.h"

static constexpr auto X86_MAX_VIRTUAL_MEMORY_REGIONS = 512;

typedef struct {
    U16 totalMapped;
    U16 indirectMapped;
} PageMetaData;

typedef struct PageMetaDataNode {
    struct PageMetaDataNode *children;
    PageMetaData metaData;
} PageMetaDataNode;

extern VirtualPageTable *rootPageTable;
extern PageMetaDataNode rootPageMetaData;

U64 getPhysicalAddressFrame(U64 virtualPage);

#endif
