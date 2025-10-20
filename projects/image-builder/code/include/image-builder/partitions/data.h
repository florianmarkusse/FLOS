#ifndef IMAGE_BUILDER_PARTITIONS_DATA_H
#define IMAGE_BUILDER_PARTITIONS_DATA_H

#include "shared/types/numeric.h"

[[nodiscard]] bool dataPartitionWrite(U8 *fileBuffer, int kernelfd,
                                      U32 kernelSizeBytes);

#endif
