// The mouse over the window: hover and clicks for the title bar buttons,
// pane close buttons and the empty workspace button; dragging dividers
// (ui/dividers.h) and pane headers (ui/header_drag.h); focus, selection,
// wheel and right click over panes (right click copies the selection, or
// pastes when nothing is selected). Coordinates are client pixels.
#pragma once

#include <stdbool.h>

#include "input/mouse_report.h"
#include "ui/app.h"

void mouse_move(App *a, int x, int y);
void mouse_leave(App *a);
void mouse_down(App *a, MouseButton b, int x, int y, bool double_click);
// buttons is the MK_* state after this release.
void mouse_up(App *a, MouseButton b, int x, int y, WPARAM buttons);
void mouse_wheel(App *a, int delta, int screen_x, int screen_y);
// WM_SETCURSOR in the client area (mouse_cursor.c).
void mouse_set_cursor(App *a);
// WM_CAPTURECHANGED: a divider or header drag ends without dropping.
void mouse_capture_lost(App *a);
