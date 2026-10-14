// The mouse wheel, counted in whole notches (a touchpad sends smaller
// steps that add up): over the sidebar its lists scroll; over a pane,
// history, arrow keys or reports (pane_wheel).
#include "pane/pane_mouse.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"
#include "ui/sidebar.h"

void mouse_wheel(App *a, int delta, int screen_x, int screen_y) {
  POINT pt = {screen_x, screen_y};
  ScreenToClient(a->hwnd, &pt);
  a->wheel_rest += delta;
  int notches = a->wheel_rest / WHEEL_DELTA;
  a->wheel_rest -= notches * WHEEL_DELTA;
  if (notches == 0 || sidebar_wheel(a, notches, pt.x, pt.y)) return;
  Pane *p = panes_at(&a->panes, pt.x, pt.y);
  if (!p) return;
  pane_wheel(p, notches, pt.x, pt.y, keyboard_mods());
  app_redraw(a);
}
