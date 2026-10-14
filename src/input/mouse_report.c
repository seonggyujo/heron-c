// Mouse tracking reports, xterm style.
#include "input/mouse_report.h"

#include <stdio.h>

bool mouse_wanted(int mode, MouseAction action, bool button_down) {
  if (mode == 0) return false;
  if (action != MOUSE_MOTION) return true;
  return mode == 1003 || (mode == 1002 && button_down);
}

int mouse_report(MouseButton button, MouseAction action, int col, int row, KeyMods mods, bool sgr,
                 char out[32]) {
  int code = (int)button;
  if (mods.shift) code += 4;
  if (mods.alt) code += 8;
  if (mods.ctrl) code += 16;
  if (action == MOUSE_MOTION) code += 32;
  if (sgr) {
    return sprintf(out, "\x1b[<%d;%d;%d%c", code, col + 1, row + 1,
                   action == MOUSE_RELEASE ? 'm' : 'M');
  }
  // X10 cannot say which button was released.
  if (action == MOUSE_RELEASE) code = (code & ~3) | 3;
  if (col + 33 > 255 || row + 33 > 255) return 0;
  out[0] = 0x1b;
  out[1] = '[';
  out[2] = 'M';
  out[3] = (char)(32 + code);
  out[4] = (char)(33 + col);
  out[5] = (char)(33 + row);
  return 6;
}
