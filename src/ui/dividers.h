// The gaps between panes as grab handles (Heron's layout/dividers.ts). A
// thin line shows in the gap on hover and while dragging, the cursor turns
// into a resize arrow, dragging moves the boundary and a double click makes
// the two sides equal. Either one switches to manual layout.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

// Each returns true when it handled the event.
bool dividers_mouse_down(App *a, int x, int y, bool double_click);
bool dividers_mouse_move(App *a, int x, int y);
bool dividers_mouse_up(App *a);
// The mouse capture was lost: the drag ends where it is.
void dividers_cancel(App *a);
// Sets the resize cursor over a divider or while dragging one.
bool dividers_set_cursor(App *a, int x, int y);
void dividers_paint(App *a);
