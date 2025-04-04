
#include "x86/memory/physical.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/kernel-parameters.h" // for KernelMemory
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h" // for U64, U32, U8

// Coming into this, All the memory is identity mapped and sorted from least
// number of pages to most number of pages Having to do some boostrapping here
// with the base page frame physical manager.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {}
