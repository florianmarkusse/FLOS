#ifndef ABSTRACTION_EFI_H
#define ABSTRACTION_EFI_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/numeric.h"

// Sets the root page table addreses so we can map memory in EFI land too. The
// mapping will only take effect once we set CR3 to that value, i.e., enter the
// kernel
void virtualMemoryRootPageInit();
void kernelMemoryManagementInit(U64 startingAddress, U64 endingAddress);

void archParamsFill(void *archParams, U64 memoryVirtualAddressAvailable);

typedef struct KernelParameters KernelParameters;
void kernelJump(U64 newStackPointer, U16 processorID,
                    KernelParameters *kernelParameters);

[[nodiscard]] U64 kernelCodeStart();
[[nodiscard]] U64 kernelCodeSizeMax();
[[nodiscard]] U64 kernelVirtualMemoryEnd();

#endif
