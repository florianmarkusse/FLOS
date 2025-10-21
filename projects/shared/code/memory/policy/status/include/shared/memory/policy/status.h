#ifndef SHARED_MEMORY_POLICY_STATUS_H
#define SHARED_MEMORY_POLICY_STATUS_H

#include "shared/types/numeric.h"

void memoryManagementStatusAppend();

typedef struct {
    U64 memory;
    U32 addresses;
} AvailableMemoryState;

[[nodiscard]] AvailableMemoryState physicalMemoryAvailableGet();
[[nodiscard]] AvailableMemoryState virtualMemoryAvailableGet();

#endif
