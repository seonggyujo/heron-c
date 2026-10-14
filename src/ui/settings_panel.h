// The settings panel under the gear button: theme, language, font size,
// animations, terminal herons, close on exit 0, reopen last work, Claude
// Code hooks (with the reason under it when turning them on or off
// failed), and Reset (which asks first). Every change applies at once and
// is remembered. Opening and closing is ui/popover.c.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void settings_paint(App *a);
// Where the open panel is.
Box settings_panel_box(App *a);
// The tooltip of the control under (x, y) and its box, NULL for none.
const char *settings_tip_at(App *a, int x, int y, Box *box);
// True when (x, y) is inside the panel.
bool settings_mouse_move(App *a, int x, int y);
bool settings_mouse_down(App *a, int x, int y);
// TIMER_RESET_BUTTON: the Reset button's "Sure?" or "Done" ran out.
void settings_reset_timer(App *a);
