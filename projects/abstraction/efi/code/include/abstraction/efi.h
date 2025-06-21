#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/types/numeric.h"

void initVirtualMemory(U64 startingAddress, U64 endingAddress,
                       PackedMemoryAllocator *virtualMemoryTree, Arena scratch);

typedef struct {
    U64 bytes;
    U64 align;
} ArchParamsRequirements;
ArchParamsRequirements initArchitecture(Arena scratch);

void fillArchParams(void *archParams);

void jumpIntoKernel(U64 newStackPointer,
                    PackedKernelParameters *kernelParameters);
U64 initScreenMemory(U64 physicalScreenAddress, U64 bytes);

#ifdef X86
#include "x86/efi-to-kernel/definitions.h"
#else
#error ABSTRACTION_EFI_H
#endif

#endif
