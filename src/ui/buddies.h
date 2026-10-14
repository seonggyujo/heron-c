// The terminal herons in the panes: for each pane whose coding agent is
// known, as many herons as Settings > Terminal herons asks (a crew each,
// buddy/crew.h), moved before every frame and drawn over the pane's
// terminal, on blank cells only. The terminal's content is never touched.
// Settings > Animations off turns them off too.
#pragma once

#include "ui/app.h"

// Before a frame: brings herons to a pane whose agent came, moves every
// heron to now, and drops those whose pane closed or whose agent left.
void buddies_step(App *a, ULONGLONG now);
// Draws p's crew over its terminal (inside the pane's drawing transform).
void buddies_paint(App *a, const Pane *p, ULONGLONG now);
// How soon a crew needs the next frame, 0 for none.
unsigned buddies_wait(const App *a, ULONGLONG now);
