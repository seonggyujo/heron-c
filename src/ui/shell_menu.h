// The shell picker that drops down from the split button's arrow, drawn in
// the app's own theme (a native popup menu would be light). Installed shells
// are listed with a check on the current one. Picking one only selects and
// remembers it; the + button opens the pane. Opening and closing is
// ui/popover.c.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void shell_menu_paint(App *a);
// Where the open menu is.
Box shell_menu_panel(App *a);
// True when (x, y) is inside the menu.
bool shell_menu_mouse_move(App *a, int x, int y);
bool shell_menu_mouse_down(App *a, int x, int y);
// Up, Down and Enter. True when used.
bool shell_menu_key(App *a, WPARAM vk);
