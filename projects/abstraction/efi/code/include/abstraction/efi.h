#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/types/numeric.h"

void initVirtualMemory(U64 startingAddress, U64 endingAddress,
                       PackedMemoryTree *virtualMemoryTree, Arena scratch);

typedef struct {
    U64 tscFrequencyPerMicroSecond;
    U64 virtualMetaDataAddress;
} ArchitectureInit;

ArchitectureInit initArchitecture(Arena scratch);
void jumpIntoKernel(U64 newStackPointer,
                    PackedKernelParameters *kernelParameters);
U64 initScreenMemory(U64 physicalScreenAddress, U64 bytes);

#endif
