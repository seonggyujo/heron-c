// The one dropdown panel open at a time: the shell menu, the settings
// panel or the "New agent" folder menu. Each has its own files; this one
// routes the window's mouse and keys to whichever is open, and closes it
// on a press outside, Esc, or when the window loses focus or changes size.
// A panel comes in over 120 ms (ease-out) from 4 px higher, at 0.98 of its
// size and transparent, growing from its top corner by the button; closing
// plays it backwards (Heron's shell-menu.css, settings.css).
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void popover_toggle(App *a, PopoverKind kind);
void popover_close(App *a);
void popover_paint(App *a);
// A panel is still coming or going (or a settings switch sliding), so
// frames must keep coming.
bool popover_busy(const App *a, ULONGLONG now);

// True when the mouse is over the open panel.
bool popover_mouse_move(App *a, int x, int y);
// True when (x, y) is inside the open panel.
bool popover_contains(App *a, int x, int y);
// True when the press was used: inside the panel, or on the button that
// opened it (which only closes it). A press elsewhere closes the panel and
// is left to the window, so it can open another one or focus a pane.
bool popover_mouse_down(App *a, int x, int y);
bool popover_key(App *a, WPARAM vk);
