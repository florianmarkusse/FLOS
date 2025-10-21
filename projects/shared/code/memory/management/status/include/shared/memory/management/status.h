#ifndef SHARED_MEMORY_MANAGEMENT_STATUS_H
#define SHARED_MEMORY_MANAGEMENT_STATUS_H

#include "shared/memory/management/definitions.h"
#include "shared/types/numeric.h"

void physicalMemoryManagerStatusAppend();
void virtualMemoryManagerStatusAppend();

void memoryAppend(Memory memory);
void mappingMemoryAppend(U64 virtualAddress, U64 physicalAddress, U64 size);
void mappingVirtualGuardPageAppend(U64 virtualAddress, U64 size);

#endif
