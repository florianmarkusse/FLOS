#include "shared/memory/policy/status.h"
#include "shared/memory/allocator/buddy.h"
#include "shared/memory/allocator/status/buddy.h"
#include "shared/memory/allocator/status/node.h"
#include "shared/memory/management/page.h"
#include "shared/memory/management/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"

// TODO: move to common/basic.c? and then use here and in test code and in
// buddystatusappend
// static U32 nodesCount(RedBlackNode *tree) {
//     if (!tree) {
//         return 0;
//     }
//
//     RedBlackNode *buffer[2 * RB_TREE_MAX_HEIGHT];
//     U32 result = 0;
//
//     buffer[0] = tree;
//     U32 len = 1;
//     while (len > 0) {
//         RedBlackNode *node = buffer[len - 1];
//         len--;
//         result++;
//
//         for (RedBlackDirection dir = 0; dir < RB_TREE_CHILD_COUNT; dir++) {
//             if (node->children[dir]) {
//                 buffer[len] = node->children[dir];
//                 len++;
//             }
//         }
//     }
//
//     return result;
// }

void appendPhysicalMemoryManagerStatus() { buddyStatusAppend(&buddyPhysical); }

void appendVirtualMemoryManagerStatus() { buddyStatusAppend(&buddyVirtual); }

static AvailableMemoryState getAvailableMemory(Buddy *buddy) {
    AvailableMemoryState result = {0};
    for (U8 i = 0; i < buddyOrderCount(buddy); i++) {
        U32 addresses = buddy->data.blocks[i].len;
        result.addresses += addresses;
        result.memory += addresses * buddyBlockSize(buddy, i);
    }
    return result;
}

AvailableMemoryState getAvailablePhysicalMemory() {
    return getAvailableMemory(&buddyPhysical);
}
AvailableMemoryState getAvailableVirtualMemory() {
    return getAvailableMemory(&buddyVirtual);
}

void memoryAppend(Memory memory) {
    INFO(STRING("["));
    INFO((void *)memory.start);
    INFO(STRING(", "));
    INFO((void *)memory.start + memory.bytes);
    INFO(STRING("]"));
}

void mappingMemoryAppend(U64 virtualAddress, U64 physicalAddress, U64 size) {
    memoryAppend((Memory){.start = virtualAddress, .bytes = size});
    INFO(STRING(" -> "));
    memoryAppend((Memory){.start = physicalAddress, .bytes = size});
    INFO(STRING(" size: "));
    INFO(size, .flags = NEWLINE);
}

void mappingVirtualGuardPageAppend(U64 virtualAddress, U64 size) {
    memoryAppend((Memory){.start = virtualAddress, .bytes = size});
    INFO(STRING(" -> [GUARD PAGE MAPPING, GUARD PAGE MAPPING] size: "));
    INFO(size, .flags = NEWLINE);
}
