// Drawing the whole window on WM_PAINT: title bar, the empty workspace or
// the panes (closed ones fading out under the rest), the divider line, the
// drop preview of a dragged pane, and the open dropdown on top.
#pragma once

#include "pane/pane_paint.h"
#include "ui/app.h"

void app_paint(App *a);
// How panes are drawn at now: fonts and sizes.
PaneStyle paint_pane_style(const App *a, ULONGLONG now);
