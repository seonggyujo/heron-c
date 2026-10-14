// Where the panes go (Heron's layout/split-layout.ts), no window. Automatic
// mode rebuilds the split tree from the grid rules on every add and close.
// The first pane drag or divider move switches to manual mode: adding then
// splits the focused pane along its longer side, and closing lets the
// neighbours take the space. One pane left, or tidy, goes back to auto.
// Items are pane ids; "live" lists are the panes on screen in the order
// they were opened.
#pragma once

#include "base/json.h"
#include "layout/split_rects.h"

typedef struct {
  SplitNode *root;
  bool manual;
  Rects rects;  // from the last arrange_place
  // Changes whenever nodes may have been freed, so a divider drag that
  // holds a split can tell it is gone.
  unsigned generation;
} Arrangement;

// live includes item. In manual mode item takes half of near (or of the
// last placed item when near is not placed).
void arrange_add(Arrangement *a, int item, int near, const int *live, int count);
// live no longer includes item.
void arrange_remove(Arrangement *a, int item, const int *live, int count);
// A dropped pane: split target on zone's side, or swap the two.
void arrange_move(Arrangement *a, int item, int target, Side zone);
void arrange_tidy(Arrangement *a, const int *live, int count);
// A divider was dragged or evened: the sizes are the user's now.
void arrange_set_manual(Arrangement *a, bool manual);
// The user's own arrangement to save (layout/tree_shape.h), with items as
// positions in live; NULL in automatic mode.
Json *arrange_shape(const Arrangement *a, const int *live, int count);
// Puts back a saved arrangement; false when it does not fit live.
bool arrange_restore(Arrangement *a, const Json *shape, const int *live, int count);

// Fills rects for the workspace, with gap pixels between panes and around
// the edge.
void arrange_place(Arrangement *a, Box workspace, int gap);
void arrange_free(Arrangement *a);
