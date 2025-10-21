#include "shared/allocator/pool.h"

#include "shared/allocator/macros.h"
#include "shared/assert.h" // for ASSERT
#include "shared/manipulation/manipulation.h"

void freePool(PoolAllocator *pool) {
    U64 chunkCount = pool->cap / pool->chunkSize;
    U64 i;

    for (i = 0; i < chunkCount; i++) {
        void *ptr = &pool->beg[i * pool->chunkSize];
        PoolHead *node = (PoolHead *)ptr;
        node->next = pool->head;
        pool->head = node;
    }
}

/*
 * Set up the pool allocator values, except for the jmp_buf!
 */
PoolAllocator createPoolAllocator(I8 *buffer, I64 cap, I64 chunkSize) {
    ASSERT(cap > 0);
    ASSERT((cap & (cap - 1)) == 0);

    ASSERT(chunkSize > 0);
    ASSERT((chunkSize & (chunkSize - 1)) == 0);
    ASSERT(chunkSize > sizeof(PoolHead));

    ASSERT(cap > chunkSize);

    PoolAllocator result;

    result.beg = buffer;
    result.cap = cap;
    result.chunkSize = chunkSize;

    result.head = nullptr;

    freePool(&result);

    return result;
}

void *poolAlloc(PoolAllocator *pool, U8 flags) {
    PoolHead *node = pool->head;

    if (node == nullptr) {
        ASSERT(false);
        if (flags & NULLPTR_ON_FAIL) {
            return nullptr;
        }
        longjmp(pool->jmp_buf, 1);
    }

    pool->head = pool->head->next;

    return flags & ALLOCATOR_ZERO_MEMORY ? memset(node, 0, pool->chunkSize) : node;
}

void freePoolNode(PoolAllocator *pool, void *ptr) {
    ASSERT((void *)pool->beg <= ptr && ptr < (void *)(pool->beg + pool->cap));

    PoolHead *node = (PoolHead *)ptr;
    node->next = pool->head;
    pool->head = node;
}
