// Moving the boundary between two neighbours of a split by dragging the gap
// between them (Heron's layout/dividers.ts), no window. Neither neighbour
// gets smaller than DIVIDER_MIN_PX; a double click makes the two equal.
#pragma once

#include "layout/split_rects.h"

#define DIVIDER_MIN_PX 80

typedef struct {
  SplitNode *split;  // NULL when no drag is going on
  int index;
  Axis axis;
  int origin;  // pointer position along the axis at the press
  int span;
  double first, pair, min;  // sizes at the press, and the smallest allowed
  unsigned generation;      // of the arrangement the split belongs to
} DividerDrag;

// Starts dragging d at pos (x for AXIS_ROW, y for AXIS_COLUMN). False when
// the split has no room to share.
bool divider_drag_begin(DividerDrag *drag, const Divider *d, int pos, unsigned generation);
// The pointer is at pos. Returns true when the sizes changed.
bool divider_drag_move(DividerDrag *drag, int pos);
// Gives the two neighbours of d equal shares. Returns true when that
// changed anything.
bool divider_equalize(const Divider *d);
