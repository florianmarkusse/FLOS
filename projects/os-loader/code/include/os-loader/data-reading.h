#ifndef OS_LOADER_DATA_READING_H
#define OS_LOADER_DATA_READING_H

#include "efi/firmware/base.h" // for Lba
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h" // for U32, U64, USize

[[nodiscard]] String kernelFromCurrentLoadedImageRead(U32 bytes, Arena scratch);

[[nodiscard]] U32 kernelBytesFromPartition(Arena scratch);

#endif
