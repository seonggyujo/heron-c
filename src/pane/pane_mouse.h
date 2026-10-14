// Mouse input over a pane's cells. Normally the left button selects text
// (double click selects a word) and the wheel scrolls history. When the
// program tracks the mouse, events go to it instead; Shift keeps them here.
// Coordinates are window client pixels.
#pragma once

#include <stdbool.h>

#include "input/mouse_report.h"
#include "pane/pane.h"

bool pane_tracks_mouse(const Pane *p, KeyMods mods);

void pane_mouse_down(Pane *p, MouseButton b, int x, int y, KeyMods mods, bool double_click);
void pane_mouse_move(Pane *p, int x, int y, KeyMods mods);
void pane_mouse_up(Pane *p, MouseButton b, int x, int y, KeyMods mods);

// notches > 0 scrolls up (back in time).
void pane_wheel(Pane *p, int notches, int x, int y, KeyMods mods);
