// Tooltips, as Heron's title attributes: the tidy and settings buttons,
// the theme swatches, the hooks switch and Reset in the settings panel,
// the folder and title in a pane header (their full text), the sidebar's
// buttons, rows and sessions, the folder menu, and the usage bar's
// context chips. A Windows tooltip control
// shows them after the mouse rests. The thing under the mouse is the one
// tool rectangle, so moving onto another starts the wait over.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void tooltip_create(App *a);
void tooltip_mouse_move(App *a, int x, int y);
// WM_NOTIFY: hands the tooltip its text. True when it was the tooltip's.
bool tooltip_notify(App *a, LPARAM lp);
