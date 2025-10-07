#include "shared/memory/allocator/status/buddy.h"
#include "shared/log.h"
#include "shared/memory/allocator/buddy.h"

void buddyStatusAppend(Buddy *buddy) {
    U64 bytesTotal = 0;
    U32 iterations =
        buddy->data.blockSizeLargest - buddy->data.blockSizeSmallest + 1;
    U64 blockSize = (1 << buddy->data.blockSizeSmallest);
    for (U32 i = 0; i < iterations; i++, blockSize *= 2) {
        INFO(STRING("order: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(i), 2));
        INFO(STRING(" block size: "));
        INFO(stringWithMinSizeDefault(CONVERT_TO_STRING(blockSize), 20));
        INFO(
            stringWithMinSizeDefault(CONVERT_TO_STRING((void *)blockSize), 19));

        U64 nodesFreeCount = buddy->data.blocks[i].len;
        INFO(STRING("["));
        INFO(stringWithMinSizeDefault(
            CONVERT_TO_STRING(buddy->data.blocks[i].len), 3));
        INFO(STRING("/"));
        INFO(stringWithMinSizeDefault(
            CONVERT_TO_STRING(buddy->data.blocksCapacityPerOrder), 3));
        INFO(STRING("]\n"));

        bytesTotal += blockSize * nodesFreeCount;
    }

    INFO(STRING("Total bytes: "));
    INFO(bytesTotal, .flags = NEWLINE);
}
