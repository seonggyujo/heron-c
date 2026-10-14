// The mouse and keys on the sidebar: hover (an agent row outlines its
// pane), presses that act on release over the same part, the right click
// that asks in a recent session's row before deleting it (a press
// elsewhere or Esc cancels; a failure shows there for 2.5 s), and the
// wheel over either list.
#include <string.h>

#include "app/prefs.h"
#include "app/strings.h"
#include "ui/hooks_state.h"
#include "ui/new_agent.h"
#include "ui/popover.h"
#include "ui/recent.h"
#include "ui/sidebar.h"
#include "ui/sidebar_hit.h"
#include "ui/sidebar_recent.h"

#define NOTE_MS 2500
#define WHEEL_STEP 60  // CSS px a notch scrolls

static SidebarHit hit_at(App *a, int x, int y, bool *inside) {
  SidebarLayout l;
  sidebar_layout(a, &l);
  Box box;
  *inside = box_contains(l.frame, x, y);
  return sidebar_hit(a, &l, x, y, &box);
}

static void set_hover(App *a, SidebarHit h) {
  int pane = h.part == SIDEBAR_AGENT ? h.index : 0;
  SidebarHit *now = &a->sidebar.hover;
  if (now->part == h.part && now->index == h.index && a->agents.hover_pane == pane) return;
  *now = h;
  a->agents.hover_pane = pane;
  app_redraw(a);
}

bool sidebar_mouse_move(App *a, int x, int y) {
  bool inside;
  set_hover(a, hit_at(a, x, y, &inside));
  return inside;
}

void sidebar_mouse_leave(App *a) { set_hover(a, (SidebarHit){SIDEBAR_NONE, 0}); }

static void close_question(App *a) {
  SidebarState *s = &a->sidebar;
  s->asking[0] = 0;
  s->note = NULL;
  app_redraw(a);
}

static void deleted(App *a, SessionDelete result) {
  SidebarState *s = &a->sidebar;
  s->deleting = false;
  if (result == SESSION_DELETED) {
    close_question(a);
    return;
  }
  s->note = result == SESSION_RUNNING ? strings()->delete_running : strings()->delete_failed;
  s->note_until = GetTickCount64() + NOTE_MS;
}

static void hide_note(const char *pref, bool *hidden) {
  *hidden = true;
  prefs_set_bool(pref, true);
}

static void run(App *a, SidebarHit h) {
  SidebarState *s = &a->sidebar;
  switch (h.part) {
    case SIDEBAR_TOGGLE: sidebar_toggle(a); break;
    case SIDEBAR_NEW: new_agent_start(a, new_agent_path(a), false); break;
    case SIDEBAR_NEW_MENU: popover_toggle(a, POPOVER_AGENT_FOLDERS); break;
    case SIDEBAR_AGENT: {
      Pane *p = panes_find(&a->panes, h.index);
      if (p) app_focus_pane(a, p);
      break;
    }
    case SIDEBAR_GROUP: recent_toggle_fold(a, h.index); break;
    case SIDEBAR_SESSION: recent_open(a, &a->agents.shown[h.index]); break;
    case SIDEBAR_ASK_YES:
      s->deleting = true;
      recent_delete(a, a->agents.shown[h.index].id, deleted);
      break;
    case SIDEBAR_ASK_NO: close_question(a); break;
    case SIDEBAR_HOOKS_ON: hooks_set(a, true); break;
    case SIDEBAR_OFFER_CLOSE: hide_note(PREF_HOOKS_OFFER_HIDDEN, &s->offer_hidden); break;
    case SIDEBAR_PLUGIN_CLOSE: hide_note(PREF_PLUGIN_HINT_HIDDEN, &s->plugin_hidden); break;
    default: break;
  }
}

bool sidebar_mouse_down(App *a, MouseButton b, int x, int y) {
  SidebarState *s = &a->sidebar;
  bool inside;
  SidebarHit h = hit_at(a, x, y, &inside);
  bool on_question = h.part == SIDEBAR_ASK || h.part == SIDEBAR_ASK_YES || h.part == SIDEBAR_ASK_NO;
  if (s->asking[0] && !s->deleting && !on_question) close_question(a);
  if (!inside) return false;
  if (b == MOUSE_RIGHT && h.part == SIDEBAR_SESSION && !s->deleting) {
    strcpy(s->asking, a->agents.shown[h.index].id);
    s->note = NULL;
    app_redraw(a);
  } else if (b == MOUSE_LEFT && h.part != SIDEBAR_NONE && h.part != SIDEBAR_ASK) {
    s->pressed = h;
    SetCapture(a->hwnd);
    app_redraw(a);
  }
  return true;
}

bool sidebar_mouse_up(App *a, int x, int y) {
  SidebarHit pressed = a->sidebar.pressed;
  if (pressed.part == SIDEBAR_NONE) return false;
  a->sidebar.pressed = (SidebarHit){SIDEBAR_NONE, 0};
  ReleaseCapture();
  bool inside;
  SidebarHit h = hit_at(a, x, y, &inside);
  if (h.part == pressed.part && h.index == pressed.index) run(a, h);
  app_redraw(a);
  return true;
}

static int clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

bool sidebar_wheel(App *a, int notches, int x, int y) {
  SidebarLayout l;
  sidebar_layout(a, &l);
  if (!box_contains(l.frame, x, y)) return false;
  int step = app_px(a, WHEEL_STEP) * notches;
  SidebarState *s = &a->sidebar;
  if (box_contains(l.list, x, y)) {
    s->agents_scroll = clamp(l.scroll - step, 0, l.rows_h > l.list.h ? l.rows_h - l.list.h : 0);
  } else if (l.recent.h && box_contains(l.recent, x, y)) {
    RecentList r;
    recent_list_layout(a, l.recent, &r);
    int most = r.content_h > r.view.h ? r.content_h - r.view.h : 0;
    s->recent_scroll = clamp(r.scroll - step, 0, most);
    s->recent_newest = s->recent_scroll >= most - app_px(a, 2);
  }
  sidebar_mouse_move(a, x, y);
  app_redraw(a);
  return true;
}

bool sidebar_key(App *a, WPARAM vk) {
  if (vk != VK_ESCAPE || !a->sidebar.asking[0] || a->sidebar.deleting) return false;
  close_question(a);
  return true;
}
