
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

typedef MAX_LENGTH_ARRAY(RedBlackNode) RedBlackNode_max_a;

static RedBlackNode *tree;
static Arena allocatable;
static RedBlackNode_max_a freeList;

static Memoory allocInternal(U64 bytes) {}

Memory allocPhysicalMemory(U64 bytes) {}

// TODO: Make this architecture agnostic?
U64 getPageForMappingVirtualMemory() {
    // TODO: THIS!
    return allocContiguousPhysicalPages(1, X86_4KIB_PAGE);
}

// NOTE: Coming into this, All the memory is identity mapped. Having to do some
// boostrapping here.
void initPhysicalMemoryManager(KernelMemory kernelMemory) {
    tree = kernelMemory.tree;

    allocatable = kernelMemory.allocator;
    if (setjmp(allocatable.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }
}
