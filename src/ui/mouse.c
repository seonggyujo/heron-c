// Mouse routing between the title bar, the empty workspace, the dividers
// and the panes.
#include "ui/mouse.h"

#include "pane/pane_mouse.h"
#include "pane/pane_paint.h"
#include "ui/dividers.h"
#include "ui/empty_state.h"
#include "ui/header_drag.h"
#include "ui/keyboard.h"
#include "ui/links.h"
#include "ui/popover.h"
#include "ui/sidebar.h"
#include "ui/titlebar.h"
#include "ui/tooltip.h"
#include "ui/usage_bar.h"

static HitTarget target_at(App *a, int x, int y) {
  HitTarget hit = titlebar_hit(a, x, y);
  if (hit == HIT_NONE && a->panes.shown.count == 0 && box_contains(empty_state_button(a), x, y)) {
    hit = HIT_EMPTY_NEW;
  }
  return hit;
}

static int close_button_at(App *a, int x, int y) {
  Pane *p = panes_at(&a->panes, x, y);
  return p && box_contains(pane_close_box(p, &a->m.pane), x, y) ? p->id : 0;
}

static void run_target(App *a, HitTarget hit) {
  switch (hit) {
    case HIT_NEW:
    case HIT_EMPTY_NEW: app_open_pane(a); break;
    case HIT_SHELL_MENU: popover_toggle(a, POPOVER_SHELLS); break;
    case HIT_SETTINGS: popover_toggle(a, POPOVER_SETTINGS); break;
    case HIT_TIDY: app_tidy_panes(a); break;
    case HIT_MIN: ShowWindow(a->hwnd, SW_MINIMIZE); break;
    case HIT_MAX: ShowWindow(a->hwnd, IsZoomed(a->hwnd) ? SW_RESTORE : SW_MAXIMIZE); break;
    case HIT_CLOSE: PostMessageW(a->hwnd, WM_CLOSE, 0, 0); break;
    default: break;
  }
}

static Pane *drag_pane(App *a) { return a->drag_pane ? panes_find(&a->panes, a->drag_pane) : NULL; }

void mouse_move(App *a, int x, int y) {
  if (!a->leave_tracked) {
    TRACKMOUSEEVENT t = {sizeof t, TME_LEAVE, a->hwnd, 0};
    a->leave_tracked = TrackMouseEvent(&t);
  }
  if (dividers_mouse_move(a, x, y) || header_drag_move(a, x, y)) return;
  tooltip_mouse_move(a, x, y);
  if (popover_mouse_move(a, x, y)) return;
  bool on_bars = sidebar_mouse_move(a, x, y) | usage_bar_mouse_move(a, x, y);
  HitTarget hit = target_at(a, x, y);
  int close = close_button_at(a, x, y);
  if (hit != a->hover || close != a->hover_close) {
    a->hover = hit;
    a->hover_close = close;
    app_redraw(a);
  }
  Pane *p = drag_pane(a);
  if (!p && on_bars) return;
  if (!p) {
    p = panes_at(&a->panes, x, y);
    for (int i = 0; i < a->panes.shown.count; i++) {
      Pane *other = a->panes.shown.items[i];
      if (other != p && other->has_hover_link) links_mouse_move(a, other, -1, -1);
    }
    if (p) links_mouse_move(a, p, x, y);
    if (p && y < p->body.y) return;  // over a header: nothing for the program
  }
  if (!p) return;
  pane_mouse_move(p, x, y, keyboard_mods());
  if (p->selecting) app_redraw(a);
}

void mouse_leave(App *a) {
  a->leave_tracked = false;
  sidebar_mouse_leave(a);
  a->chip_hover = 0;
  if (a->hover == HIT_NONE && !a->hover_close && a->divider_hover < 0) return;
  a->hover = HIT_NONE;
  a->hover_close = 0;
  a->divider_hover = -1;
  app_redraw(a);
}

void mouse_down(App *a, MouseButton b, int x, int y, bool double_click) {
  if (popover_mouse_down(a, x, y) || sidebar_mouse_down(a, b, x, y)) return;
  if (b == MOUSE_LEFT && usage_bar_click(a, x, y)) return;
  HitTarget hit = target_at(a, x, y);
  if (hit != HIT_NONE) {
    if (b == MOUSE_LEFT) {
      a->pressed = hit;
      SetCapture(a->hwnd);
      app_redraw(a);
    }
    return;
  }
  if (b == MOUSE_LEFT && dividers_mouse_down(a, x, y, double_click)) return;
  Pane *p = panes_at(&a->panes, x, y);
  if (!p) return;
  if (b == MOUSE_LEFT && close_button_at(a, x, y) == p->id) {
    app_close_pane(a, p);
    return;
  }
  app_focus_pane(a, p);
  if (y < p->body.y - a->m.pane.pad_top) {  // the header: focus, and maybe a drag
    if (b == MOUSE_LEFT) header_drag_press(a, p, x, y);
    return;
  }
  a->drag_pane = p->id;
  SetCapture(a->hwnd);
  if (b == MOUSE_LEFT && links_mouse_down(a, p, x, y)) return;
  KeyMods mods = keyboard_mods();
  if (b == MOUSE_RIGHT && !pane_tracks_mouse(p, mods)) return;  // acts on release
  pane_mouse_down(p, b, x, y, mods, double_click);
  app_redraw(a);
}

void mouse_up(App *a, MouseButton b, int x, int y, WPARAM buttons) {
  if (b == MOUSE_LEFT && sidebar_mouse_up(a, x, y)) return;
  if (a->pressed != HIT_NONE) {
    HitTarget pressed = a->pressed;
    a->pressed = HIT_NONE;
    ReleaseCapture();
    if (target_at(a, x, y) == pressed) run_target(a, pressed);
    app_redraw(a);
    return;
  }
  if (b == MOUSE_LEFT && (dividers_mouse_up(a) || header_drag_release(a))) return;
  Pane *p = drag_pane(a);
  if (p && links_mouse_up(a, p, x, y)) {
    p = NULL;
  } else if (p) {
    KeyMods mods = keyboard_mods();
    if (b == MOUSE_RIGHT && !pane_tracks_mouse(p, mods)) {
      if (!keyboard_copy(a, p)) keyboard_paste(a, p);
    } else {
      pane_mouse_up(p, b, x, y, mods);
    }
    app_redraw(a);
  }
  if (!(buttons & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON))) {
    a->drag_pane = 0;
    ReleaseCapture();
  }
}

void mouse_capture_lost(App *a) {
  dividers_cancel(a);
  header_drag_cancel(a);
}
