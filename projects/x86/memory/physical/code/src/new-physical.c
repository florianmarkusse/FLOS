
#include "shared/memory/sizes.h"
#include "shared/trees/red-black.h"
#include "x86/memory/physical.h"

#include "abstraction/interrupts.h"
#include "abstraction/jmp.h"
#include "abstraction/memory/manipulation.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi-to-kernel/kernel-parameters.h" // for KernelMemory
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h" // for U64, U32, U8

static constexpr auto TREE_SIZE = 16 * KiB;
static constexpr auto FREE_LIST_TREE_SIZE = 16 * KiB;

static RedBlackNode *tree;
static Arena allocatable;

// NOTE: Coming into this, All the memory is identity mapped. Having to do some
// boostrapping here with the base page frame physical manager.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    tree = kernelMemory.tree;

    allocatable = kernelMemory.allocator;
    if (setjmp(allocatable.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }
}
