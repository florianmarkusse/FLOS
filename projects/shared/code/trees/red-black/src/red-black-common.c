#include "shared/trees/red-black-common.h"

RedBlackDirection calculateDirection(U64 value, U64 toCompare) {
    if (value >= toCompare) {
        return RB_TREE_RIGHT;
    }
    return RB_TREE_LEFT;
}
