#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "shared/types/numeric.h"
#include "x86/memory/definitions.h"
static constexpr auto X86_MAX_VIRTUAL_MEMORY_REGIONS = 512;

typedef struct {
    U16 entriesMapped;
    U16 entriesMappedWithSmallerGranularity;
} PageMetaData;

typedef struct PageMetaDataNode {
    struct PageMetaDataNode *children;
    PageMetaData metaData;
} PageMetaDataNode;

typedef struct __attribute__((packed)) {
    U16 entriesMapped;
    U16 entriesMappedWithSmallerGranularity;
} PackedPageMetaData;

typedef struct __attribute__((packed)) {
    struct PackedPageMetaDataNode *children;
    PackedPageMetaData metaData;
} PackedPageMetaDataNode;

extern VirtualPageTable *rootPageTable;
extern PageMetaDataNode rootPageMetaData;

U64 getPhysicalAddressFrame(U64 virtualPage);

VirtualPageTable *getZeroedPageTable();

#endif
