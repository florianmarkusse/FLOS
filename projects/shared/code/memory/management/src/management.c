#include "shared/memory/management/management.h"

#include "abstraction/interrupts.h"
#include "efi-to-kernel/kernel-parameters.h"
#include "shared/maths/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/trees/red-black/memory-manager.h"

MemoryAllocator virt;
MemoryAllocator physical;

static void insertMemory(Memory memory, MemoryAllocator *allocator) {
    RedBlackNodeMM *newNode;
    if (allocator->freeList.len > 0) {
        newNode = allocator->freeList.buf[allocator->freeList.len - 1];
        allocator->freeList.len--;
    } else {
        newNode = NEW(&allocator->arena, RedBlackNodeMM);
    }
    newNode->memory = memory;

    InsertResult insertResult = insertRedBlackNodeMM(&allocator->tree, newNode);
    for (U64 i = 0; (i < RED_BLACK_MM_MAX_POSSIBLE_FREES_ON_INSERT) &&
                    insertResult.freed[i];
         i++) {
        allocator->freeList.buf[allocator->freeList.len] =
            insertResult.freed[i];
        allocator->freeList.len++;
    }
}

// The remaining bytes are put back into the red black tree or we have a perfect
// match and the node's memory location can be now put in the free list
static void handleFreeMemory(RedBlackNodeMM *availableMemory, U64 bytes,
                             MemoryAllocator *allocator) {
    availableMemory->memory.bytes -= bytes;
    if (availableMemory->memory.bytes) {
        availableMemory->memory.start += bytes;
        insertRedBlackNodeMM(&allocator->tree, availableMemory);
    } else {
        allocator->freeList.buf[allocator->freeList.len] = availableMemory;
        allocator->freeList.len++;
    }
}

static RedBlackNodeMM *getMemoryNode(U64 bytes, RedBlackNodeMM **tree) {
    RedBlackNodeMM *availableMemory = deleteAtLeastRedBlackNodeMM(tree, bytes);
    if (!availableMemory) {
        interruptNoMorePhysicalMemory();
    }
    return availableMemory;
}

void freeVirtualMemory(Memory memory) { insertMemory(memory, &virt); }

void freePhysicalMemory(Memory memory) { insertMemory(memory, &physical); }

static void *allocAlignedMemory(U64 bytes, U64 align,
                                MemoryAllocator *allocator) {
    bytes = ALIGN_UP_VALUE(bytes, align);
    RedBlackNodeMM *availableMemory = getMemoryNode(bytes, &allocator->tree);
    U64 result = ALIGN_UP_VALUE(availableMemory->memory.start, align);

    handleFreeMemory(availableMemory, bytes, allocator);
    if (result > availableMemory->memory.start) {
        availableMemory->memory.bytes = result - availableMemory->memory.start;
        insertMemory(availableMemory->memory, allocator);
    }

    return (void *)result;
}

void *allocVirtualMemory(U64 bytes, U64 align) {
    return allocAlignedMemory(bytes, align, &virt);
}

void *allocPhysicalMemory(U64 bytes, U64 align) {
    return allocAlignedMemory(bytes, align, &physical);
}

static void setupArena(PackedArena packed, Arena *arena) {
    arena->beg = packed.beg;
    arena->curFree = packed.curFree;
    arena->end = packed.end;
}

static U64 getRequiredFreeListSize(MemoryAllocator *allocator) {
    U64 redBlackNodeMMsPossibleInAllocator =
        (allocator->arena.end - allocator->arena.beg) /
        sizeof(*(allocator->tree));
    return redBlackNodeMMsPossibleInAllocator * sizeof(allocator->tree);
}

void initVirtualMemoryManager(PackedMemoryTree virtualMemoryTree) {
    setupArena(virtualMemoryTree.allocator, &virt.arena);
    if (setjmp(virt.arena.jmp_buf)) {
        interruptNoMoreVirtualMemory();
    }

    virt.tree = virtualMemoryTree.tree;

    U64 freeListRequiredSize = getRequiredFreeListSize(&virt);
    virt.freeList = (RedBlackNodeMMPtr_a){
        .len = 0,
        .buf = allocPhysicalMemory(freeListRequiredSize,
                                   alignof(*virt.freeList.buf))};
}

// NOTE: Coming into this, All the memory is identity mapped. Having to do some
// boostrapping here.
void initPhysicalMemoryManager(PackedMemoryTree physicalMemoryTree) {
    setupArena(physicalMemoryTree.allocator, &physical.arena);
    if (setjmp(physical.arena.jmp_buf)) {
        interruptNoMorePhysicalMemory();
    }

    physical.tree = physicalMemoryTree.tree;

    U64 freeListRequiredSize = getRequiredFreeListSize(&physical);

    RedBlackNodeMM *availableMemory =
        getMemoryNode(freeListRequiredSize, &physical.tree);
    physical.freeList = (RedBlackNodeMMPtr_a){
        .len = 0, .buf = (RedBlackNodeMM **)availableMemory->memory.start};
    handleFreeMemory(availableMemory, freeListRequiredSize, &physical);
}
