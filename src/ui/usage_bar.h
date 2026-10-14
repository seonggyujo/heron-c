// The bar along the bottom of the window, from the heron-limits plugin
// (Heron's ui/usage-bar.ts and limit-meter.ts):
//   5-hour limit  resets in 2h 13m | Weekly limit  resets in 4d 2h | Context
//   58% ━━━━━━────                 | 21% ━━────────              | [1 heron 34%]
// A meter per known usage limit, then a chip per agent with its context
// use, in pane order; clicking a chip goes to that pane. Numbers and bars
// turn yellow from 70% and red from 90%. Hidden while there is neither a
// limit nor a context figure; the panes make room when it comes and goes.
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "ui/app.h"

// 64 px, 0 while hidden.
int usage_bar_height(const App *a);
// After the board changed: shows or hides the bar.
void usage_bar_sync(App *a);
void usage_bar_paint(App *a);

// True when (x, y) is on the bar.
bool usage_bar_mouse_move(App *a, int x, int y);
bool usage_bar_click(App *a, int x, int y);
const char *usage_bar_tip_at(App *a, int x, int y, Box *box, char *buf, size_t cap);
