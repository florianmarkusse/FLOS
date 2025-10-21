#ifndef SHARED_TYPES_ARRAY_H
#define SHARED_TYPES_ARRAY_H

#include "shared/types/numeric.h"

#define ARRAY(T)                                                               \
    struct {                                                                   \
        T *buf;                                                                \
        U32 len;                                                               \
    }

#define ARRAY_MAX_LENGTH(T)                                                    \
    struct {                                                                   \
        T *buf;                                                                \
        U32 len;                                                               \
        U32 cap;                                                               \
    }

#endif
