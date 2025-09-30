#include "shared/memory/allocator/status/node.h"
#include "shared/log.h"

void_max_a nodes;
voidPtr_max_a nodesFreeList;
U32 elementSizeBytes;
U32 alignBytes;

void nodeAllocatorStatusAppend(NodeAllocator *nodeAllocator) {
    INFO(STRING("buffer["));
    INFO(nodeAllocator->nodes.len);
    INFO(STRING("/"));
    INFO(nodeAllocator->nodes.cap);
    INFO(STRING("], freeList["));
    INFO(nodeAllocator->nodesFreeList.len);
    INFO(STRING("/"));
    INFO(nodeAllocator->nodesFreeList.cap);
    INFO(STRING("], element bytes: "));
    INFO(nodeAllocator->elementSizeBytes);
    INFO(STRING(" element align: "));
    INFO(nodeAllocator->alignBytes, .flags = NEWLINE);
}
