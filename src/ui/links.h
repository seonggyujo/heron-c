// Links in terminal output (Heron's terminal/links.ts): the link under the
// mouse is underlined, and Ctrl+click opens http and https links in the
// default browser, like Windows Terminal. A Ctrl+click on a link is kept
// from a program that tracks the mouse, which would otherwise open it a
// second time (Claude Code also cuts the URL at the first non-ASCII letter).
#pragma once

#include <stdbool.h>

#include "ui/app.h"

// Updates the underlined link. Returns true when (x, y) is over a link.
bool links_mouse_move(App *a, Pane *p, int x, int y);
// Ctrl+left press on a link: remembered for the release, and used.
bool links_mouse_down(App *a, Pane *p, int x, int y);
// Release after links_mouse_down: opens the link if still over it.
bool links_mouse_up(App *a, Pane *p, int x, int y);
