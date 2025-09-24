#include "shared/memory/policy/status.h"
#include "shared/memory/management/page.h"
#include "shared/memory/management/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/memory/management/management.h"
#include "shared/text/string.h"

static void countAvailable(MMNode *current, U64 *available, U32 *nodes) {
    if (!current) {
        return;
    }

    countAvailable(current->children[RB_TREE_LEFT], available, nodes);

    *available += current->memory.bytes;
    *nodes += 1;

    countAvailable(current->children[RB_TREE_RIGHT], available, nodes);
}

static void appendMemoryManagerStatus(RedBlackMMTreeWithFreeList *allocator,
                                      String name) {
    INFO(name);
    AvailableMemoryState result = {0};
    countAvailable(allocator->tree, &result.memory, &result.nodes);
    INFO(STRING(" mem: "));
    INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(result.memory), 16));
    INFO(STRING(" nodes: "));
    INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(result.nodes), 3));
    INFO(STRING("freelist size: "));
    INFO(stringWithMinSizeDefault(
        CONVERT_TO_STRING(allocator->nodeAllocator.nodesFreeList.len), 3));
    INFO(STRING("nodes buf: "));
    INFO((void *)allocator->nodeAllocator.nodes.buf);
    INFO(STRING(" len: "));
    INFO(stringWithMinSizeDefault(
        CONVERT_TO_STRING(allocator->nodeAllocator.nodes.len), 3));
    INFO(STRING(" cap: "));
    INFO(allocator->nodeAllocator.nodes.cap, .flags = NEWLINE);
}

void appendPhysicalMemoryManagerStatus() {
    appendMemoryManagerStatus(&physicalMA, STRING("[PHYS]"));
}

void appendVirtualMemoryManagerStatus() {
    appendMemoryManagerStatus(&virtualMA, STRING("[VIRT]"));
}

void memoryVirtualGuardPageStatusAppend() {
    U8 *buffer = virtualMemorySizeMapper.nodeAllocator.nodes.buf;
    for (typeof(virtualMemorySizeMapper.nodeAllocator.nodes.len) i = 0;
         i < virtualMemorySizeMapper.nodeAllocator.nodes.len; i++) {
        VMMNode *node =
            (VMMNode *)(buffer + (virtualMemorySizeMapper.nodeAllocator
                                      .elementSizeBytes *
                                  i));
        if (!node->mappingSize) {
            mappingVirtualGuardPageAppend(node->basic.value, node->bytes);
        }
    }
}

static AvailableMemoryState getAvailableMemory(MMNode *tree) {
    AvailableMemoryState result = {0};
    countAvailable(tree, &result.memory, &result.nodes);
    return result;
}

AvailableMemoryState getAvailablePhysicalMemory() {
    return getAvailableMemory(physicalMA.tree);
}
AvailableMemoryState getAvailableVirtualMemory() {
    return getAvailableMemory(virtualMA.tree);
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
