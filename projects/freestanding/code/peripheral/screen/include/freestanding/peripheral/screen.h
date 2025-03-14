#ifndef FREESTANDING_PERIPHERAL_SCREEN_H
#define FREESTANDING_PERIPHERAL_SCREEN_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/array-types.h"
#include "shared/types/types.h" // for U32, U8, U64, I64, I8, U16

// This struct implicitly assumes that there are 4 bytes per pixel, hence a
// uint32 buffer
typedef struct {
    U32 *screen;
    U32 *backingBuffer;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} ScreenDimension;
void initScreen(ScreenDimension dimension, Arena *perm);
// TODO: needs buffer as argument when memory is set up
void rewind(U16 numberOfScreenLines);
void prowind(U16 numberOfScreenLines);
bool flushToScreen(U8_max_a buffer);

#endif
