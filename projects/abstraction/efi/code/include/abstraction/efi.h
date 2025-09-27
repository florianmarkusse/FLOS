#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/numeric.h"

// Sets the root page table addreses so we can map memory in EFI land too. The
// mapping will only take effect once we set CR3 to that value, i.e., enter the
// kernel
void initRootVirtualMemoryInKernel();
void initKernelMemoryManagement(U64 startingAddress, U64 endingAddress);

void fillArchParams(void *archParams, Arena scratch,
                    U64 memoryVirtualAddressAvailable);

typedef struct KernelParameters KernelParameters;
void jumpIntoKernel(U64 newStackPointer, U16 processorID,
                    KernelParameters *kernelParameters);
U64 initScreenMemory(U64 physicalScreenAddress, U64 bytes);

U64 kernelCodeStart();
U64 kernelCodeSizeMax();
U64 kernelVirtualMemoryEnd();

#endif
