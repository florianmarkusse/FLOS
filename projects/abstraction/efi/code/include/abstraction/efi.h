#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/types/types.h"

void initArchitecture(Arena scratch);
void jumpIntoKernel(U64 newStackPointer, KernelParameters *kernelParameters);
U64 initScreenMemory(U64 physicalScreenAddress, U64 bytes);

#endif
