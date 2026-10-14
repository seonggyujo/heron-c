// Dragging a pane by its header onto another pane (Heron's
// pane/pane-drag.ts): near an edge it splits that pane on that side, in the
// middle the two swap places. Past 5 px of travel the dragged pane turns
// translucent and a box shows where it will go. Esc cancels; the key never
// reaches the shell.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void header_drag_press(App *a, Pane *p, int x, int y);
// Each returns true when it handled the event.
bool header_drag_move(App *a, int x, int y);
bool header_drag_release(App *a);
bool header_drag_key(App *a, WPARAM vk);
// The mouse capture was lost: nothing is dropped.
void header_drag_cancel(App *a);

// p is being dragged (drawn translucent).
bool header_drag_dragging(const App *a, const Pane *p);
// The grab cursor while dragging.
bool header_drag_set_cursor(App *a);
void header_drag_paint(App *a);
