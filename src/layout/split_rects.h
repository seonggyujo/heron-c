// Turning a split tree into pixel boxes (Heron's layout/split-rects.ts), no
// window. Children of a split share its length minus the gaps between them.
// Edges are rounded so panes sit on whole pixels (sharp terminal text); a
// gap may then be 1 px off. The gaps between children are the dividers the
// user drags.
#pragma once

#include "base/box.h"
#include "layout/split_tree.h"

typedef struct {
  int item;
  Box box;
} ItemBox;

// The gap between child index and index + 1 of split.
typedef struct {
  SplitNode *split;
  int index;
  Axis axis;
  Box box;
  int span;  // pixels the children of split share, gaps excluded
} Divider;

typedef struct {
  ItemBox *boxes;
  int box_count;
  Divider *dividers;
  int divider_count;
} Rects;

// Replaces what r held.
void rects_compute(Rects *r, SplitNode *root, Box area, int gap);
void rects_free(Rects *r);
// The box of item; false when it is not placed.
bool rects_box_of(const Rects *r, int item, Box *out);
