#ifndef FREESTANDING_MEMORY_MANIPULATION_H
#define FREESTANDING_MEMORY_MANIPULATION_H

#include "shared/types/numeric.h"

/* Copy N bytes of SRC to DEST.  */
__attribute((nothrow, nonnull(1, 2))) void *
memcpy(void *__restrict dest, const void *__restrict src, U64 n);
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
__attribute((nothrow, nonnull(1, 2))) void *memmove(void *dest, const void *src,
                                                    U64 n);

/* Set N bytes of S to C.  */
__attribute((nothrow, nonnull(1))) void *memset(void *s, int c, U64 n);

/* Compare N bytes of S1 and S2.  */
__attribute((nothrow, pure, nonnull(1, 2))) int memcmp(const void *s1,
                                                       const void *s2, U64 n);

#endif
