#ifndef SHARED_MEMORY_POLICY_STATUS_H
#define SHARED_MEMORY_POLICY_STATUS_H

#include "shared/types/numeric.h"

void appendMemoryManagementStatus();

typedef struct {
    U64 memory;
    U32 addresses;
} AvailableMemoryState;

AvailableMemoryState getAvailablePhysicalMemory();
AvailableMemoryState getAvailableVirtualMemory();

#endif
