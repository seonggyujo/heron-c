// Divider hover, drag, double click and line.
#include "ui/dividers.h"

#include "app/theme.h"
#include "base/log.h"

static const Rects *rects(const App *a) { return &a->panes.arrangement.rects; }

static int divider_at(const App *a, int x, int y) {
  for (int i = 0; i < rects(a)->divider_count; i++) {
    if (box_contains(rects(a)->dividers[i].box, x, y)) return i;
  }
  return -1;
}

static int along(Axis axis, int x, int y) { return axis == AXIS_ROW ? x : y; }

// The divider being dragged, found again after a layout.
static const Divider *dragged(const App *a) {
  for (int i = 0; i < rects(a)->divider_count; i++) {
    const Divider *d = &rects(a)->dividers[i];
    if (d->split == a->divider.split && d->index == a->divider.index) return d;
  }
  return NULL;
}

static void resized(App *a) {
  PaneLayout l = app_pane_layout(a);
  panes_resized(&a->panes, &l);
  app_redraw(a);
}

bool dividers_mouse_down(App *a, int x, int y, bool double_click) {
  int i = divider_at(a, x, y);
  if (i < 0) return false;
  const Divider *d = &rects(a)->dividers[i];
  if (double_click) {
    if (divider_equalize(d)) {
      log_debug("dividers", "split %d|%d made equal", d->index, d->index + 1);
      resized(a);
    }
    return true;
  }
  if (divider_drag_begin(&a->divider, d, along(d->axis, x, y), a->panes.arrangement.generation)) {
    SetCapture(a->hwnd);
    app_redraw(a);
  }
  return true;
}

bool dividers_mouse_move(App *a, int x, int y) {
  if (a->divider.split) {
    // A pane opened or closed under the drag: its split may be gone.
    if (a->divider.generation != a->panes.arrangement.generation) {
      dividers_cancel(a);
      return true;
    }
    if (divider_drag_move(&a->divider, along(a->divider.axis, x, y))) resized(a);
    return true;
  }
  int hover = divider_at(a, x, y);
  if (hover != a->divider_hover) {
    a->divider_hover = hover;
    app_redraw(a);
  }
  return false;
}

bool dividers_mouse_up(App *a) {
  if (!a->divider.split) return false;
  if (a->divider.generation == a->panes.arrangement.generation) {
    log_debug("dividers", "split %d|%d at %.3f of %.3f", a->divider.index, a->divider.index + 1,
              a->divider.split->sizes[a->divider.index], a->divider.pair);
  }
  a->divider.split = NULL;
  ReleaseCapture();
  app_redraw(a);
  return true;
}

void dividers_cancel(App *a) {
  if (!a->divider.split) return;
  a->divider.split = NULL;
  ReleaseCapture();
  app_redraw(a);
}

bool dividers_set_cursor(App *a, int x, int y) {
  Axis axis;
  if (a->divider.split) {
    axis = a->divider.axis;
  } else {
    int i = divider_at(a, x, y);
    if (i < 0) return false;
    axis = rects(a)->dividers[i].axis;
  }
  SetCursor(LoadCursorW(NULL, axis == AXIS_ROW ? IDC_SIZEWE : IDC_SIZENS));
  return true;
}

void dividers_paint(App *a) {
  const Divider *d = a->divider.split ? dragged(a) : NULL;
  if (!d && a->divider_hover >= 0 && a->divider_hover < rects(a)->divider_count) {
    d = &rects(a)->dividers[a->divider_hover];
  }
  if (!d) return;
  float line = 2 * a->scale;
  Box b = d->box;
  if (d->axis == AXIS_ROW) {
    canvas_fill_round(a->canvas, b.x + (b.w - line) / 2, (float)b.y, line, (float)b.h, line / 2, THEME_BORDER_STRONG);
  } else {
    canvas_fill_round(a->canvas, (float)b.x, b.y + (b.h - line) / 2, (float)b.w, line, line / 2, THEME_BORDER_STRONG);
  }
}
