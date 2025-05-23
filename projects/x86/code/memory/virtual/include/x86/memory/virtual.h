#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

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

typedef struct {
    U16 entriesMapped;
    U16 entriesMappedWithSmallerGranularity;
} __attribute__((packed)) PackedPageMetaData;

typedef struct PackedPageMetaDataNode {
    struct PackedPageMetaDataNode *children;
    PackedPageMetaData metaData;
} __attribute__((packed)) PackedPageMetaDataNode;

extern VirtualPageTable *rootPageTable;
extern PageMetaDataNode rootPageMetaData;

U64 getPhysicalAddressFrame(U64 virtualPage);

#endif
