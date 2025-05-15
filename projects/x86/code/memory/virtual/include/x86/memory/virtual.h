#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "x86/memory/definitions.h"

static constexpr auto X86_MAX_VIRTUAL_MEMORY_REGIONS = 512;

typedef struct VirtualMetaData {
    U16 count;
    struct VirtualMetaData **pages;
} VirtualMetaData;

typedef struct {
    U64 pages[PageTableFormat.ENTRIES];
} VirtualReferenceCount;

extern VirtualPageTable *rootPageTable;
extern VirtualMetaData *rootVirtualMetaData;
extern U64 rootReferenceCount;

U64 getPhysicalAddressFrame(U64 virtualPage);

#endif
