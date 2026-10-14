// The terminal herons in the panes: a crew (buddy/crew.h) for each pane
// whose coding agent is known, moved before every frame and drawn over the
// pane's terminal, on blank cells only. The terminal's content is never
// touched. Settings > Terminal herons, and Animations, turn them off.
#pragma once

#include "ui/app.h"

// Before a frame: makes a crew for a pane whose agent came, moves every
// crew to now, and drops those whose pane closed or whose agent left.
void buddies_step(App *a, ULONGLONG now);
// Draws p's crew over its terminal (inside the pane's drawing transform).
void buddies_paint(App *a, const Pane *p, ULONGLONG now);
// How soon a crew needs the next frame, 0 for none.
unsigned buddies_wait(const App *a, ULONGLONG now);
