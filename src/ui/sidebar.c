// The sidebar's remembered choices, its gliding width and its clock.
#include "ui/sidebar.h"

#include <math.h>

#include "app/prefs.h"
#include "base/easing.h"
#include "base/log.h"
#include "ui/animation.h"

#define GLIDE_MS 160  // agent-sidebar.css's width transition, ease-out

void sidebar_load(App *a) {
  SidebarState *s = &a->sidebar;
  s->collapsed = prefs_get_bool(PREF_SIDEBAR_COLLAPSED, false);
  s->offer_hidden = prefs_get_bool(PREF_HOOKS_OFFER_HIDDEN, false);
  s->plugin_hidden = prefs_get_bool(PREF_PLUGIN_HINT_HIDDEN, false);
  if (!prefs_get(PREF_FOLDED_GROUPS, s->folded, sizeof s->folded)) s->folded[0] = 0;
  s->recent_newest = true;
}

void sidebar_reset(App *a) {
  bool collapsed = a->sidebar.collapsed;
  sidebar_load(a);
  a->sidebar.collapsed = collapsed;
  if (collapsed) sidebar_toggle(a);
}

int sidebar_target_width(const App *a) { return app_px(a, a->sidebar.collapsed ? 48 : 256); }

int sidebar_width(const App *a) {
  const SidebarState *s = &a->sidebar;
  int to = sidebar_target_width(a);
  ULONGLONG t = GetTickCount64() - s->toggled_at;
  if (!sidebar_busy(a, GetTickCount64())) return to;
  float from = (float)app_px(a, s->collapsed ? 256 : 48);
  return (int)lroundf(from + (to - from) * ease(EASE_OUT_CSS, (float)t / GLIDE_MS));
}

bool sidebar_busy(const App *a, ULONGLONG now) {
  return a->motion && a->sidebar.toggled_at && now - a->sidebar.toggled_at < GLIDE_MS;
}

void sidebar_toggle(App *a) {
  SidebarState *s = &a->sidebar;
  s->collapsed = !s->collapsed;
  s->toggled_at = GetTickCount64();
  s->hover = (SidebarHit){0};
  a->agents.hover_pane = 0;
  prefs_set_bool(PREF_SIDEBAR_COLLAPSED, s->collapsed);
  log_info("sidebar", "%s", s->collapsed ? "collapsed" : "expanded");
  app_layout(a);  // the panes follow the width as it glides (ui/paint.c)
  anim_after_paint(a);
  app_redraw(a);
}

static bool counting(const App *a) {
  for (int i = 0; i < a->agents.board.count; i++) {
    const BoardEntry *e = &a->agents.board.entries[i];
    if (!e->slot.present) continue;
    if (e->slot.agent.state == AGENT_WORKING || e->slot.agent.subagent_count) return true;
  }
  return false;
}

void sidebar_tick(App *a) {
  SidebarState *s = &a->sidebar;
  bool redraw = counting(a) && !s->collapsed;
  if (s->note && GetTickCount64() >= s->note_until) {
    s->note = NULL;
    s->asking[0] = 0;
    redraw = true;
  }
  if (redraw) app_redraw(a);
}
