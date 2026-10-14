// Where a dragged pane lands on another pane (Heron's layout/drop-zone.ts),
// no window. The outer quarter on each side splits the target on that side
// (corners go to the nearer edge); the middle swaps the two panes.
#pragma once

#include "base/box.h"
#include "layout/split_tree.h"

Side drop_zone_at(Box target, int x, int y);

// The part of the target the dragged pane would take: a half, or all of it
// for a swap.
Box drop_preview(Box target, Side zone);
