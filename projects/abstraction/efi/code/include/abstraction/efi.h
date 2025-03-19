#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/types.h"

void initArchitecture(Arena scratch);
void jumpIntoKernel(U64 newStackPointer);

#endif
