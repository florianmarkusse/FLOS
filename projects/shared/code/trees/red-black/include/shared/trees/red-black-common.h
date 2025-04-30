#ifndef SHARED_TREES_RED_BLACK_COMMON_H
#define SHARED_TREES_RED_BLACK_COMMON_H

typedef enum { RB_TREE_BLACK = 0, RB_TREE_RED = 1 } RedBlackColor;
typedef enum { RB_TREE_LEFT = 0, RB_TREE_RIGHT = 1 } RedBlackDirection;

static constexpr auto RB_TREE_CHILD_COUNT = 2;
static constexpr auto RB_TREE_MAX_HEIGHT = 128;

#endif
