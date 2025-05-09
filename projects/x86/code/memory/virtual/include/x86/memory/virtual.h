#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "x86/memory/definitions.h"

static constexpr auto X86_MAX_VIRTUAL_MEMORY_REGIONS = 512;
extern VirtualPageTable *rootPageTable;

U8 pageSizeToDepth(PageSize pageSize);

U64 getPhysicalAddressFrame(U64 virtualPage);

#endif
