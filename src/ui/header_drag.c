// Header press, drag, drop and the drop preview.
#include "ui/header_drag.h"

#include <math.h>

#include "app/theme.h"
#include "base/log.h"
#include "layout/drop_zone.h"

#define THRESHOLD_PX 5

void header_drag_press(App *a, Pane *p, int x, int y) {
  a->header_drag = (HeaderDrag){p->id, x, y, false, 0, SIDE_CENTER};
  SetCapture(a->hwnd);
}

bool header_drag_move(App *a, int x, int y) {
  HeaderDrag *d = &a->header_drag;
  if (!d->pane) return false;
  if (!d->moving) {
    if (hypot(x - d->start_x, y - d->start_y) < THRESHOLD_PX * a->scale) return true;
    d->moving = true;
    log_debug("header-drag", "dragging pane %d", d->pane);
  }
  Pane *target = panes_at(&a->panes, x, y);
  d->target = target && target->id != d->pane ? target->id : 0;
  if (d->target) d->zone = drop_zone_at(target->frame, x, y);
  app_redraw(a);
  return true;
}

static void finish(App *a) {
  bool moving = a->header_drag.moving;
  a->header_drag = (HeaderDrag){0};
  ReleaseCapture();
  if (moving) app_redraw(a);
}

bool header_drag_release(App *a) {
  HeaderDrag d = a->header_drag;
  if (!d.pane) return false;
  finish(a);
  Pane *p = panes_find(&a->panes, d.pane), *target = d.target ? panes_find(&a->panes, d.target) : NULL;
  if (!d.moving) return true;
  if (!p || !target) {
    log_debug("header-drag", "drag of pane %d cancelled", d.pane);
    return true;
  }
  log_info("header-drag", "dropped pane %d on pane %d (zone %d)", d.pane, d.target, (int)d.zone);
  app_move_pane(a, p, target, d.zone);
  return true;
}

bool header_drag_key(App *a, WPARAM vk) {
  if (!a->header_drag.moving || vk != VK_ESCAPE) return false;
  log_debug("header-drag", "drag of pane %d cancelled", a->header_drag.pane);
  finish(a);
  return true;
}

void header_drag_cancel(App *a) {
  if (a->header_drag.pane) finish(a);
}

bool header_drag_dragging(const App *a, const Pane *p) {
  return a->header_drag.moving && a->header_drag.pane == p->id;
}

bool header_drag_set_cursor(App *a) {
  if (!a->header_drag.moving) return false;
  SetCursor(LoadCursorW(NULL, IDC_SIZEALL));
  return true;
}

// Half of the target, or all of it for a swap (dashed).
void header_drag_paint(App *a) {
  const HeaderDrag *d = &a->header_drag;
  Pane *target = d->moving && d->target ? panes_find(&a->panes, d->target) : NULL;
  if (!target) return;
  Box b = drop_preview(target->frame, d->zone);
  float r = (float)a->m.pane.radius;
  canvas_fill_round(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h, r, THEME_GLOW);
  if (d->zone == SIDE_CENTER) {
    canvas_stroke_round_dashed(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h, r, 1, THEME_FG_DIM);
  } else {
    canvas_stroke_round(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h, r, 1, THEME_FG_DIM);
  }
}
