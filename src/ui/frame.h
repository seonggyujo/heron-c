// The window frame drawn by the app instead of Windows: the whole window is
// client area, with resize edges, a draggable title bar, Windows 11 rounded
// corners and shadow, and a dark border.
#pragma once

#include "ui/app.h"

void frame_init(HWND hwnd);
// The thin border Windows draws around the window, in the theme's color.
void frame_apply_theme(HWND hwnd);
LRESULT frame_calc_size(HWND hwnd, WPARAM wp, LPARAM lp);
LRESULT frame_hit_test(App *a, LPARAM lp);
void frame_min_size(App *a, MINMAXINFO *info);
