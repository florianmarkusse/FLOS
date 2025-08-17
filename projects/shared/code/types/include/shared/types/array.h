#ifndef SHARED_TYPES_ARRAY_H
#define SHARED_TYPES_ARRAY_H

#include "shared/types/numeric.h"

#define ARRAY(T)                                                               \
    struct {                                                                   \
        T *buf;                                                                \
        U32 len;                                                               \
    }

#define PACKED_MAX_LENGTH_ARRAY(T)                                             \
    struct __attribute__((packed)) {                                           \
        T *buf;                                                                \
        U32 len;                                                               \
        U32 cap;                                                               \
    }

#define MAX_LENGTH_ARRAY(T)                                                    \
    struct {                                                                   \
        T *buf;                                                                \
        U32 len;                                                               \
        U32 cap;                                                               \
    }

#endif
