#ifndef OS_LOADER_DATA_READING_H
#define OS_LOADER_DATA_READING_H

#include "efi/firmware/base.h" // for Lba
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/types/types.h" // for U32, U64, USize

typedef struct {
    U64 bytes;
    U64 lbaStart;
} DataPartitionFile;

string readKernelFromCurrentLoadedImage(U64 bytes, Arena scratch);

U64 getKernelBytes(Arena scratch);

#endif
