// The mouse cursor for what is under it: grab and resize arrows while
// dragging, an arrow over a dropdown, a hand over a link, an I-beam over
// terminal text.
#include "pane/pane_mouse.h"
#include "ui/dividers.h"
#include "ui/header_drag.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"
#include "ui/popover.h"

void mouse_set_cursor(App *a) {
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(a->hwnd, &pt);
  if (header_drag_set_cursor(a)) return;
  if (popover_contains(a, pt.x, pt.y)) {
    SetCursor(LoadCursorW(NULL, IDC_ARROW));
    return;
  }
  if (dividers_set_cursor(a, pt.x, pt.y)) return;
  Pane *p = panes_at(&a->panes, pt.x, pt.y);
  bool text = p && pt.y >= p->body.y && !pane_tracks_mouse(p, keyboard_mods());
  LPCWSTR shape = p && p->has_hover_link ? IDC_HAND : text ? IDC_IBEAM : IDC_ARROW;
  SetCursor(LoadCursorW(NULL, shape));
}
