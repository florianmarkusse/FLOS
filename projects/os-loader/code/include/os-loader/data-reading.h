#ifndef OS_LOADER_DATA_READING_H
#define OS_LOADER_DATA_READING_H

#include "efi/firmware/base.h" // for Lba
#include "shared/text/string.h"
#include "shared/types/types.h" // for U32, U64, USize

typedef struct {
    U64 bytes;
    U64 lbaStart;
} DataPartitionFile;

string readDiskLbasFromCurrentGlobalImage(Lba diskLba, USize bytes);

string readDiskLbasFromEfiImage(Lba diskLba, USize bytes);

DataPartitionFile getKernelInfo();

#endif
