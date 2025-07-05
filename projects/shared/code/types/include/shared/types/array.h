#ifndef SHARED_TYPES_ARRAY_H
#define SHARED_TYPES_ARRAY_H

#include "shared/types/numeric.h"

#define PACKED_ARRAY(T)                                                        \
    struct __attribute__((packed)) {                                           \
        T *buf;                                                                \
        U64 len;                                                               \
    }

#define ARRAY(T)                                                               \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                               \
    }

#define PACKED_MAX_LENGTH_ARRAY(T)                                             \
    struct __attribute__((packed)) {                                           \
        T *buf;                                                                \
        U64 len;                                                               \
        U64 cap;                                                               \
    }

#define MAX_LENGTH_ARRAY(T)                                                    \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                               \
        U64 cap;                                                               \
    }

#endif
