#ifndef IMAGE_BUILDER_PARTITIONS_EFI_H
#define IMAGE_BUILDER_PARTITIONS_EFI_H

#include "shared/types/numeric.h"

U32 calculateEFIPartitionSize(U32 EFIApplicationSizeLBA);
bool writeEFISystemPartition(U8 *fileBuffer, int efifd, U64 efiSizeBytes,
                             U64 kernelSizeBytes);

#endif
