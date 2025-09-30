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
static U32 nodesCount(RedBlackNode *tree) {
    if (!tree) {
        return 0;
    }

    RedBlackNode *buffer[2 * RB_TREE_MAX_HEIGHT];
    U32 result = 0;

    buffer[0] = tree;
    U32 len = 1;
    while (len > 0) {
        RedBlackNode *node = buffer[len - 1];
        len--;
        result++;

        for (RedBlackDirection dir = 0; dir < RB_TREE_CHILD_COUNT; dir++) {
            if (node->children[dir]) {
                buffer[len] = node->children[dir];
                len++;
            }
        }
    }

    return result;
}

// static void appendMemoryManagerStatus(RedBlackMMTreeWithFreeList *allocator,
//                                       String name) {
//     INFO(name);
//     AvailableMemoryState result = getAvailableMemory();
//     countAvailable(allocator->tree, &result.memory, &result.nodes);
//     INFO(STRING(" mem: "));
//     INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(result.memory), 16));
//     INFO(STRING(" nodes: "));
//     INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(result.nodes), 3));
//     INFO(STRING("freelist size: "));
//     INFO(stringWithMinSizeDefault(
//         CONVERT_TO_STRING(allocator->nodeAllocator.nodesFreeList.len), 3));
//     INFO(STRING("nodes buf: "));
//     INFO((void *)allocator->nodeAllocator.nodes.buf);
//     INFO(STRING(" len: "));
//     INFO(stringWithMinSizeDefault(
//         CONVERT_TO_STRING(allocator->nodeAllocator.nodes.len), 3));
//     INFO(STRING(" cap: "));
//     INFO(allocator->nodeAllocator.nodes.cap, .flags = NEWLINE);
// }

void appendPhysicalMemoryManagerStatus() {
    buddyStatusAppend(&buddyPhysical.buddy);
    nodeAllocatorStatusAppend(&buddyPhysical.nodeAllocator);
}

void appendVirtualMemoryManagerStatus() {
    buddyStatusAppend(&buddyVirtual.buddy);
    nodeAllocatorStatusAppend(&buddyVirtual.nodeAllocator);
}

void memoryVirtualGuardPageStatusAppend() {
    U8 *buffer = memoryMapperSizes.nodeAllocator.nodes.buf;
    for (typeof(memoryMapperSizes.nodeAllocator.nodes.len) i = 0;
         i < memoryMapperSizes.nodeAllocator.nodes.len; i++) {
        VMMNode *node =
            (VMMNode *)(buffer +
                        (memoryMapperSizes.nodeAllocator.elementSizeBytes * i));
        if (!node->mappingSize) {
            mappingVirtualGuardPageAppend(node->basic.value, node->bytes);
        }
    }
}

static AvailableMemoryState getAvailableMemory(Buddy *buddy) {
    AvailableMemoryState result = {0};
    for (U8 i = 0; i < buddyOrderCount(buddy); i++) {
        U32 nodes = nodesCount((RedBlackNode *)buddy->data.blocksFree[i]);
        result.nodes += nodes;
        result.memory += nodes * buddyBlockSize(buddy, i);
    }
    return result;
}

AvailableMemoryState getAvailablePhysicalMemory() {
    return getAvailableMemory(&buddyPhysical.buddy);
}
AvailableMemoryState getAvailableVirtualMemory() {
    return getAvailableMemory(&buddyVirtual.buddy);
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
    INFO(STRING(" -> [XXXXXXXXXXXXXXXXXX, XXXXXXXXXXXXXXXXXX] GUARD PAGE\n"));
}
