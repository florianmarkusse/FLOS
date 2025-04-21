#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "x86/memory/definitions.h"

extern VirtualPageTable *rootPageTable;

U8 pageSizeToDepth(PageSize pageSize);

U64 getPhysicalAddressFrame(U64 virtualPage);
MappedPage getMappedPage(U64 virt);

#endif
