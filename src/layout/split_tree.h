// The split tree that places panes (Heron's layout/split-tree.ts), no
// window. A leaf holds one item (a pane id). A split lays its children side
// by side (AXIS_ROW) or stacked (AXIS_COLUMN); each child takes its share
// of sizes (fractions that add up to 1). Operations change nodes in place
// and return the root, which may be a different node afterwards.
#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum { AXIS_ROW, AXIS_COLUMN } Axis;
typedef enum { SIDE_LEFT, SIDE_RIGHT, SIDE_TOP, SIDE_BOTTOM, SIDE_CENTER } Side;

typedef struct SplitNode {
  bool leaf;
  int item;  // leaf: the pane id
  Axis axis;  // split:
  int count;
  struct SplitNode **children;
  double *sizes;
} SplitNode;

SplitNode *tree_leaf(int item);
void tree_free(SplitNode *node);

// Heron's automatic grid as a tree: a column of rows, cols = ceil(sqrt(n)),
// rows filled in order and the last row's items stretched to the full
// width (3 items: two on top, one wide below). NULL for no items.
SplitNode *tree_auto(const int *items, int count);

// Items in reading order. Returns how many there are (up to cap written).
int tree_items(const SplitNode *root, int *out, int cap);
bool tree_contains(const SplitNode *root, int item);

// Puts item on side of target; the two share the target's space. A split
// in the same direction as the target's parent becomes its sibling.
SplitNode *tree_insert(SplitNode *root, int item, int target, Side side);
// Takes item out; its siblings grow to fill the space. NULL when it was
// the last one.
SplitNode *tree_remove(SplitNode *root, int item);
// Moves item next to target on side, or swaps the two for SIDE_CENTER.
SplitNode *tree_move(SplitNode *root, int item, int target, Side side);

// A short shape for logs, e.g. "column(row(1 2) 3)".
void tree_describe(const SplitNode *root, char *out, size_t cap);
