// Opening, closing, moving and focusing panes, with their animations: the
// other panes glide to their new boxes, a new pane fades in, and a closed
// pane fades out before its place goes to the others.
#include <stdlib.h>

#include "agent/agent_files.h"
#include "base/utf.h"
#include "pane/pane_input.h"
#include "ui/agents.h"
#include "ui/animation.h"
#include "ui/app.h"
#include "ui/pane_motion.h"

// After the panes changed places: the moved ones glide from where they
// were drawn (s), and a divider under the mouse may be another one now.
static void rearranged(App *a, MotionSnapshot *s) {
  motion_glide(a, s);
  a->divider_hover = -1;
  anim_after_paint(a);
  app_redraw(a);
}

Pane *app_open_pane_in(App *a, ShellKind shell, const wchar_t *cwd, const char *command) {
  PaneHost host = {a->hwnd, WM_PANE_OUTPUT, WM_PANE_EXIT};
  char key[AGENT_KEY_MAX];
  agent_key(panes_next_id(&a->panes), key);
  wchar_t *wkey = utf8_to_wide(key, -1), *wdir = utf8_to_wide(agent_dir() ? agent_dir() : "", -1);
  EnvVar env[2] = {{L"HERON_PANE", wkey}, {L"HERON_AGENT_DIR", wdir}};
  PaneStart start = {shell, cwd, env, agent_dir() ? 2 : 0, command};
  PaneLayout l = app_pane_layout(a);
  Pane *old = panes_focused(&a->panes);
  if (old && a->active) pane_focus_changed(old, false);
  MotionSnapshot s;
  motion_capture(a, &s);
  Pane *p = panes_open(&a->panes, &start, &host, &l);
  free(wkey);
  free(wdir);
  motion_enter(a, p);
  rearranged(a, &s);
  return p;
}

// The + button, as in Heron: the folder given on the command line, else the
// user's home.
void app_open_pane(App *a) { app_open_pane_in(a, a->shell, a->start_dir, NULL); }

void app_close_pane(App *a, Pane *p) {
  if (a->drag_pane == p->id) a->drag_pane = 0;
  if (a->header_drag.pane == p->id || a->header_drag.target == p->id) a->header_drag = (HeaderDrag){0};
  if (a->drop_pane == p->id) a->drop_pane = 0;
  a->hover_close = 0;
  agents_closed(a, p->id);
  Pane *next = panes_close(&a->panes, p);
  motion_leave(a, p);
  app_focus_pane(a, next);
  app_settle_panes(a);
  anim_after_paint(a);
  app_redraw(a);
}

void app_settle_panes(App *a) {
  ULONGLONG now = GetTickCount64();
  PaneLayout l = app_pane_layout(a);
  for (int i = a->panes.leaving.count - 1; i >= 0; i--) {
    Pane *p = a->panes.leaving.items[i];
    if (!motion_left(a, p, now)) continue;
    MotionSnapshot s;
    motion_capture(a, &s);
    panes_release(&a->panes, p, &l);
    rearranged(a, &s);
  }
}

void app_move_pane(App *a, Pane *p, Pane *target, Side zone) {
  PaneLayout l = app_pane_layout(a);
  MotionSnapshot s;
  motion_capture(a, &s);
  panes_move(&a->panes, p, target, zone, &l);
  app_focus_pane(a, p);
  rearranged(a, &s);
}

void app_tidy_panes(App *a) {
  PaneLayout l = app_pane_layout(a);
  MotionSnapshot s;
  motion_capture(a, &s);
  panes_tidy(&a->panes, &l);
  rearranged(a, &s);
}

void app_focus_pane(App *a, Pane *p) {
  Pane *old = panes_focused(&a->panes);
  if (old == p) return;
  if (old && a->active) pane_focus_changed(old, false);
  panes_focus(&a->panes, p);
  if (p && a->active) pane_focus_changed(p, true);
  if (p) agents_seen(a, p);
  a->unfocused_pane = old ? old->id : 0;  // its glow fades as p's comes
  a->focus_moved_at = GetTickCount64();
  anim_after_paint(a);
  app_redraw(a);
}

void app_focus_step(App *a, int step) {
  Pane *old = panes_focused(&a->panes);
  panes_focus_step(&a->panes, step);
  Pane *now = panes_focused(&a->panes);
  panes_focus(&a->panes, old);
  app_focus_pane(a, now);
}
