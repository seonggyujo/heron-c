// What the workspace shows with no panes: a large + button and the caption
// "new terminal_" with a blinking cursor (Heron's empty state).
#pragma once

#include <stdbool.h>

#include "ui/app.h"

Box empty_state_button(const App *a);
void empty_state_paint(App *a);
// Whether the cursor is lit at time now (GetTickCount64): 1.1 s blink.
bool empty_state_cursor_on(const App *a, unsigned long long now);
