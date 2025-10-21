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
} poolAllocatorInit;

void poolFree(poolAllocatorInit *pool);

/*
 * Set up the pool allocator values, except for the jmp_buf!
 */
[[nodiscard]] poolAllocatorInit createPoolAllocator(I8 *buffer, I64 cap,
                                                    I64 chunkSize);

[[nodiscard]] __attribute__((malloc)) void *poolAlloc(poolAllocatorInit *pool,
                                                      U8 flags);

void poolFreeNode(poolAllocatorInit *pool, void *ptr);

#endif
