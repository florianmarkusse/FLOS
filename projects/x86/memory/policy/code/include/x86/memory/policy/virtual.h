#ifndef X86_MEMORY_POLICY_VIRTUAL_H
#define X86_MEMORY_POLICY_VIRTUAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"

extern VirtualRegion higherHalfRegion;
extern VirtualRegion lowerHalfRegion; // Start is set in the init function.

void initVirtualMemoryManager(VirtualMemory virt);
U64 getVirtualMemory(U64 size, PageSize alignValue);

U64 getPhysicalAddressFrame(U64 virtualPage);
MappedPage getMappedPage(U64 virt);

#endif
