// The title bar: the brand on the left; on the right the "back to the
// automatic grid" button (only while the layout is manual), the running
// count, the settings gear, the new terminal split button
// ([+ New terminal · shell][v]) and the window buttons in Windows order
// (minimize, maximize, close) drawn as round lights.
#pragma once

#include "ui/app.h"

typedef struct {
  Box tidy, count, settings, new_button, menu_button, min, max, close;
} TitleLayout;

void titlebar_layout(App *a, TitleLayout *t);
// The button under (x, y), or HIT_NONE (which drags the window there).
HitTarget titlebar_hit(App *a, int x, int y);
void titlebar_paint(App *a);
