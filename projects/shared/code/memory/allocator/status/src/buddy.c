#include "shared/memory/allocator/status/buddy.h"
#include "shared/log.h"
#include "shared/memory/allocator/buddy.h"

// static void buddyNodesValueAppend(RedBlackNodeBasic *node) {
//     if (!node) {
//         return;
//     }
//
//     INFO(STRING("\t"));
//     INFO((void *)node->value);
//
//     buddyNodesValueAppend(node->children[RB_TREE_LEFT]);
//     buddyNodesValueAppend(node->children[RB_TREE_RIGHT]);
// }

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
            stringWithMinSizeDefault(CONVERT_TO_STRING((void *)blockSize), 20));
        U64 nodesFreeCount = 0;

        RedBlackNodeBasic *tree = buddy->data.blocksFree[i];
        if (tree) {
            RedBlackNodeBasic *buffer[2 * RB_TREE_MAX_HEIGHT];
            buffer[0] = tree;
            U32 len = 1;

            while (len > 0) {
                RedBlackNodeBasic *node = buffer[len - 1];
                len--;
                nodesFreeCount++;

                for (RedBlackDirection dir = 0; dir < RB_TREE_CHILD_COUNT;
                     dir++) {
                    if (node->children[dir]) {
                        buffer[len] = node->children[dir];
                        len++;
                    }
                }
            }
        }

        INFO(STRING("Free: "));
        INFO(nodesFreeCount, .flags = NEWLINE);
        bytesTotal += blockSize * nodesFreeCount;

        // if (tree) {
        //     buddyNodesValueAppend(tree);
        //     INFO(STRING("\n"));
        // }
    }

    INFO(STRING("Total bytes: "));
    INFO(bytesTotal, .flags = NEWLINE);
}
