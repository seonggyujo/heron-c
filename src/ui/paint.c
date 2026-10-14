// One full frame. Direct2D redraws everything each time; WM_PAINT only comes
// when the message queue is empty, so bursts of output share one frame.
#include "ui/paint.h"

#include "app/theme.h"
#include "pane/pane_paint.h"
#include "ui/agents.h"
#include "ui/animation.h"
#include "ui/dividers.h"
#include "ui/empty_state.h"
#include "ui/header_drag.h"
#include "ui/pane_motion.h"
#include "ui/popover.h"
#include "ui/sidebar.h"
#include "ui/titlebar.h"
#include "ui/usage_bar.h"

// While the sidebar's width glides, the workspace follows it, and so do
// the panes (laid out for the width it settles at): their boxes stretch
// from that workspace to the one of the moment.
static PanePose follow_sidebar(const App *a, PanePose pose) {
  Box to = app_workspace(a);
  float x = (float)sidebar_width(a);
  if ((int)x == to.x) return pose;
  float sx = ((float)(to.x + to.w) - x) / (float)to.w;
  pose.x = x + (pose.x - (float)to.x) * sx;
  pose.w *= sx;
  return pose;
}

// Draws p where its animation puts it: moved and scaled from its frame,
// and faded.
static void paint_pane(App *a, Pane *p, const PaneStyle *style, PaneState state) {
  PanePose pose = follow_sidebar(a, motion_pose(a, p, style->now));
  if (header_drag_dragging(a, p)) pose.opacity *= 0.5f;
  if (pose.opacity <= 0.01f) return;
  Box f = p->frame;
  float sx = pose.w / f.w, sy = pose.h / f.h;
  bool moved = pose.x != f.x || pose.y != f.y || pose.w != f.w || pose.h != f.h;
  if (moved) canvas_set_transform(a->canvas, sx, sy, pose.x - f.x * sx, pose.y - f.y * sy);
  if (pose.opacity < 1) canvas_push_opacity(a->canvas, pose.opacity);
  pane_paint(a->canvas, p, style, state);
  if (pose.opacity < 1) canvas_pop_opacity(a->canvas);
  if (moved) canvas_set_transform(a->canvas, 1, 1, 0, 0);
}

PaneStyle paint_pane_style(const App *a, ULONGLONG now) {
  PaneStyle style = {a->header_font, a->font, &a->m.pane, a->scale, now};
  return style;
}

static void paint_panes(App *a) {
  ULONGLONG now = GetTickCount64();
  PaneStyle style = paint_pane_style(a, now);
  for (int i = 0; i < a->panes.leaving.count; i++) {
    Pane *p = a->panes.leaving.items[i];
    PaneState state = {.dot = 1};
    paint_pane(a, p, &style, state);
  }
  Pane *focused = panes_focused(&a->panes);
  for (int i = 0; i < a->panes.shown.count; i++) {
    Pane *p = a->panes.shown.items[i];
    bool selected = p == focused;
    PaneState state = {selected,
                       anim_glow(a, p, selected, now),
                       selected && a->active,
                       a->hover_close == p->id,
                       a->drop_pane == p->id,
                       anim_flash(a, p, now),
                       anim_dot(a, p, now),
                       agents_dot_color(a, p),
                       a->agents.hover_pane == p->id,
                       selected ? a->composition : NULL};
    paint_pane(a, p, &style, state);
  }
}

void app_paint(App *a) {
  PAINTSTRUCT ps;
  BeginPaint(a->hwnd, &ps);
  if (a->font && canvas_begin(a->canvas)) {
    canvas_clear(a->canvas, THEME_BG_APP);
    titlebar_paint(a);
    float empty = anim_empty(a, GetTickCount64());
    if (empty > 0.01f) {
      if (empty < 1) canvas_push_opacity(a->canvas, empty);
      empty_state_paint(a);
      if (empty < 1) canvas_pop_opacity(a->canvas);
    }
    paint_panes(a);
    dividers_paint(a);
    sidebar_paint(a);
    usage_bar_paint(a);
    header_drag_paint(a);
    popover_paint(a);
    canvas_end(a->canvas);
  }
  EndPaint(a->hwnd, &ps);
  anim_after_paint(a);
}
