#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/types/numeric.h"

// Sets the root page table addreses so we can map memory in EFI land too. The
// mapping will only take effect once we set CR3 to that value, i.e., enter the
// kernel
void initRootVirtualMemoryInKernel();
void initKernelMemoryManagement(U64 startingAddress, U64 endingAddress,
                                Arena scratch);

typedef struct {
    U64 bytes;
    U64_pow2 align;
} ArchParamsRequirements;
ArchParamsRequirements getArchParamsRequirements();

void fillArchParams(void *archParams, Arena scratch);

void jumpIntoKernel(U64 newStackPointer, U16 processorID,
                    PackedKernelParameters *kernelParameters);
U64 initScreenMemory(U64 physicalScreenAddress, U64 bytes);

#ifdef X86
#include "x86/efi-to-kernel/definitions.h"
#else
#error ABSTRACTION_EFI_H
#endif

#endif
