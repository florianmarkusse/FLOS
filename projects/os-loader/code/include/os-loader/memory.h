#ifndef OS_LOADER_MEMORY_BOOT_FUNCTIONS_H
#define OS_LOADER_MEMORY_BOOT_FUNCTIONS_H

#include "efi-to-kernel/kernel-parameters.h"
#include "efi/firmware/base.h" // for PhysicalAddress
#include "efi/firmware/graphics-output.h"
#include "efi/firmware/memory.h"
#include "efi/memory/physical.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/management/definitions.h"
#include "shared/memory/management/management.h"
#include "shared/memory/sizes.h"
#include "shared/types/numeric.h" // for USize, U64, U32

static constexpr auto DYNAMIC_MEMORY_CAPACITY = 1 * MiB;

void kernelPhysicalBuddyPrepare(BuddyWithNodeAllocator *kernelBuddyPhysical,
                                GraphicsOutputProtocolMode *mode,
                                Arena scratch);

void convertToKernelMemory(MemoryInfo *memoryInfo,
                           BuddyWithNodeAllocator *kernelBuddyPhysical);

#endif
