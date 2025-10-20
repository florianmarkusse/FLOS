#ifndef IMAGE_BUILDER_PARTITIONS_EFI_H
#define IMAGE_BUILDER_PARTITIONS_EFI_H

#include "shared/types/numeric.h"

[[nodiscard]] U32 EFISystemPartitionSize(U32 EFIApplicationSizeLBA);
[[nodiscard]] bool EFISystemPartitionWrite(U8 *fileBuffer, int efifd,
                                           U32 efiSizeBytes,
                                           U32 kernelSizeBytes);

#endif
