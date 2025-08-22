#ifndef FREESTANDING_PERIPHERAL_SCREEN_H
#define FREESTANDING_PERIPHERAL_SCREEN_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/allocator/arena.h"
#include "shared/types/array-types.h"
#include "shared/types/numeric.h" // for U32, U8, U64, I64, I8, U16

void initScreen(PackedWindow *window, Arena *perm);
// TODO: needs buffer as argument when memory is set up
void rewind(U16 numberOfScreenLines);
void prowind(U16 numberOfScreenLines);
bool flushToScreen(U8_a buffer);

#endif
