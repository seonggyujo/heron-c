// Mouse reports for programs that track the mouse (vim, htop, Claude Code):
// which events a tracking mode wants, and the SGR (mode 1006) and classic
// X10 encodings.
#pragma once

#include <stdbool.h>

#include "input/keys.h"

typedef enum {
  MOUSE_LEFT = 0,
  MOUSE_MIDDLE = 1,
  MOUSE_RIGHT = 2,
  MOUSE_NO_BUTTON = 3,
  MOUSE_WHEEL_UP = 64,
  MOUSE_WHEEL_DOWN = 65,
} MouseButton;

typedef enum { MOUSE_PRESS, MOUSE_RELEASE, MOUSE_MOTION } MouseAction;

// mode is 1000 (clicks), 1002 (clicks and drags) or 1003 (all motion).
bool mouse_wanted(int mode, MouseAction action, bool button_down);

// col and row start at 0. Returns the byte count, or 0 when the X10 form
// cannot express the position. out must hold 32 bytes.
int mouse_report(MouseButton button, MouseAction action, int col, int row, KeyMods mods, bool sgr,
                 char out[32]);
