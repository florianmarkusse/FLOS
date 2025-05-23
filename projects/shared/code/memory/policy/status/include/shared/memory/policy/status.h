#ifndef SHARED_MEMORY_POLICY_STATUS_H
#define SHARED_MEMORY_POLICY_STATUS_H

#include "shared/types/numeric.h"

void appendMemoryManagementStatus();

U64 getAvailablePhysicalMemory();
U64 getAvailableVirtualMemory();

#endif
