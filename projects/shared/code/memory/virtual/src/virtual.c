#include "shared/memory/virtual.h"

#include "abstraction/interrupts.h"
#include "abstraction/memory/virtual/converter.h"
#include "abstraction/memory/virtual/map.h"
#include "efi-to-kernel/memory/definitions.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/memory/converter.h"
#include "shared/memory/physical.h"
#include "shared/memory/policy.h"
#include "x86/memory/definitions.h"

RedBlackNodeMM *virtualTree;
static Arena allocatable;
static RedBlackNodeMMPtr_a freeList;

void initVirtualMemoryManager(PackedMemoryTree virtualMemoryTree) {
    virtualTree = virtualMemoryTree.tree;

    allocatable.beg = virtualMemoryTree.allocator.beg;
    allocatable.curFree = virtualMemoryTree.allocator.curFree;
    allocatable.end = virtualMemoryTree.allocator.end;
    if (setjmp(allocatable.jmp_buf)) {
        interruptNoMoreVirtualMemory();
    }

    U64 redBlackNodeMMsPossibleInAllocator =
        (virtualMemoryTree.allocator.end - virtualMemoryTree.allocator.beg) /
        sizeof(*virtualTree);
    U64 freeListRequiredSize =
        redBlackNodeMMsPossibleInAllocator * sizeof(virtualTree);

    freeList = (RedBlackNodeMMPtr_a){
        .len = 0, .buf = allocPhysicalMemory(freeListRequiredSize)};
}

static RedBlackNodeMM *getMemoryNode(U64 bytes) {
    RedBlackNodeMM *availableMemory =
        deleteAtLeastRedBlackNodeMM(&physicalTree, bytes);
    if (!availableMemory) {
        interruptNoMorePhysicalMemory();
    }
    return availableMemory;
}

U64 getVirtual(U64 size, U64 align) {
    U64 sizeToFetch = size + align;
    U64 alignedUpValue = ALIGN_UP_VALUE(higherHalfRegion.start, align);
    //
    //    ASSERT(higherHalfRegion.start <= alignedUpValue);
    //    ASSERT(alignedUpValue <= higherHalfRegion.end);
    //    ASSERT(higherHalfRegion.end - (alignedUpValue + size) <
    //           higherHalfRegion.start);
    //
    //    higherHalfRegion.start = alignedUpValue;
    //    U64 result = higherHalfRegion.start;
    //
    //    higherHalfRegion.start += size;
    return 0;
}

static U64 alignVirtual(U64 virt, U64 physical, U64 bytes) {
    U64 alignment = pageSizeEncompassing(bytes * 2);

    virt = ALIGN_UP_VALUE(virt, alignment);
    virt = virt | RING_RANGE_VALUE(physical, alignment);

    return virt;
}

// U64 getVirtualForPhysical(U64 physical, U64 bytes) {
//     U64 requiredVirtual = pageSizeEncompassing(bytes);
//     if (bytes > requiredVirtual) {
//         requiredVirtual =
//             RING_RANGE_VALUE(bytes + requiredVirtual, requiredVirtual);
//     }
//
//     U64 virt;
//     for (U64 i = freeVirtualMemory.len; i-- > 0; freeVirtualMemory.len--) {
//         virt = freeVirtualMemory.buf[i].start;
//         virt = alignVirtual(virt, physical, bytes);
//
//         if (virt + requiredVirtual <= freeVirtualMemory.buf[i].end) {
//             freeVirtualMemory.buf[i].start = virt + requiredVirtual;
//             return virt;
//         }
//     }
//
//     interruptNoMoreVirtualMemory();
// }

//    for (U64 bytesMapped = 0, mappingSize; bytesMapped < bytes;
//         virt += mappingSize, physical += mappingSize,
//             bytesMapped += mappingSize) {
//        mappingSize = pageSizeLeastLargerThan(physical, bytes -
//        bytesMapped);
//
//        mapPageWithFlags(virt, physical, mappingSize, flags);
//    }
//
//    return virt;
