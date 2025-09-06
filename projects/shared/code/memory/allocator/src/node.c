#include "shared/memory/allocator/node.h"

void nodeAllocatorInit(NodeAllocator *nodeAllocator, void_a nodes,
                       void_a nodesFreeList, U32 elementSizeBytes) {
    *nodeAllocator = (NodeAllocator){
        .nodes =
            {
                .buf = nodes.buf,
                .len = 0,
                .cap = nodes.len / elementSizeBytes,
            },
        .nodesFreeList =
            {
                .buf = nodesFreeList.buf,
                .len = 0,
                .cap = nodesFreeList.len /
                       sizeof(typeof(*nodeAllocator->nodesFreeList.buf)),
            },
        .elementSizeBytes = elementSizeBytes};
}

void *nodeAllocatorGet(NodeAllocator *nodeAllocator) {
    if (nodeAllocator->nodesFreeList.len > 0) {
        void *result = nodeAllocator->nodesFreeList
                           .buf[nodeAllocator->nodesFreeList.len - 1];
        nodeAllocator->nodesFreeList.len--;
        return result;
    }

    if (nodeAllocator->nodes.len < nodeAllocator->nodes.cap) {
        U8 *base = nodeAllocator->nodes.buf;
        void *result =
            base + (nodeAllocator->elementSizeBytes * nodeAllocator->nodes.len);
        nodeAllocator->nodes.len++;
        return result;
    }

    return nullptr;
}
