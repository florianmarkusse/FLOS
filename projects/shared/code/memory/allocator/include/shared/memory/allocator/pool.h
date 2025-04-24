#ifndef SHARED_MEMORY_ALLOCATOR_POOL_H
#define SHARED_MEMORY_ALLOCATOR_POOL_H

#include "shared/types/numeric.h"

struct PoolHead {
    struct PoolHead *next;
};
typedef struct PoolHead PoolHead;

typedef struct {
    I8 *beg;
    I64 cap;
    I64 chunkSize;

    PoolHead *head;

    void **jmp_buf;
} PoolAllocator;

void freePool(PoolAllocator *pool);

/*
 * Set up the pool allocator values, except for the jmp_buf!
 */
PoolAllocator createPoolAllocator(I8 *buffer, I64 cap, I64 chunkSize);

__attribute((malloc)) void *poolAlloc(PoolAllocator *pool, U8 flags);

void freePoolNode(PoolAllocator *pool, void *ptr);

#endif
