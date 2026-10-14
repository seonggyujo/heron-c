// What part of the sidebar is under the mouse, and what it explains.
#include <stdio.h>

#include "app/strings.h"
#include "ui/agent_row.h"
#include "ui/new_agent.h"
#include "ui/sidebar.h"
#include "ui/sidebar_hints.h"
#include "ui/sidebar_hit.h"
#include "ui/sidebar_recent.h"

static SidebarHit hit(SidebarPart part, int index, Box at, Box *box) {
  *box = at;
  return (SidebarHit){part, index};
}

SidebarHit sidebar_hit(App *a, const SidebarLayout *l, int x, int y, Box *box) {
  *box = (Box){0};
  if (!box_contains(l->frame, x, y)) return (SidebarHit){SIDEBAR_NONE, 0};
  if (box_contains(l->toggle, x, y)) return hit(SIDEBAR_TOGGLE, 0, l->toggle, box);
  if (box_contains(l->new_main, x, y)) return hit(SIDEBAR_NEW, 0, l->new_main, box);
  if (l->new_arrow.w && box_contains(l->new_arrow, x, y)) return hit(SIDEBAR_NEW_MENU, 0, l->new_arrow, box);
  if (box_contains(l->list, x, y)) {
    for (int i = 0; i < l->row_count; i++) {
      if (box_contains(l->row_boxes[i], x, y)) return hit(SIDEBAR_AGENT, l->rows[i].pane, l->row_boxes[i], box);
    }
  }
  if (l->recent.h && box_contains(l->recent, x, y)) {
    RecentList recent;
    recent_list_layout(a, l->recent, &recent);
    SidebarHit h = recent_list_hit(a, &recent, x, y);
    for (int i = 0; i < recent.count && h.part != SIDEBAR_NONE; i++) {
      bool same = recent.items[i].index == h.index && recent.items[i].group == (h.part == SIDEBAR_GROUP);
      if (same) *box = recent.items[i].box;
    }
    return h;
  }
  if (l->foot.h && box_contains(l->foot, x, y)) {
    HintsLayout hints;
    int pad = app_px(a, 14);
    sidebar_hints_layout(a, l->foot.x + pad, l->foot.y + 1 + app_px(a, 12), l->foot.w - 2 * pad, &hints);
    SidebarPart part = sidebar_hints_hit(&hints, x, y);
    return hit(part, 0, part == SIDEBAR_HOOKS_ON ? hints.turn_on : hints.close, box);
  }
  return (SidebarHit){SIDEBAR_NONE, 0};
}

Box sidebar_new_agent_box(App *a) {
  SidebarLayout l;
  sidebar_layout(a, &l);
  Box m = l.new_main;
  if (l.new_arrow.w) m.w = l.new_arrow.x + l.new_arrow.w - m.x;
  return m;
}

bool sidebar_menu_button_at(App *a, int x, int y) {
  SidebarLayout l;
  sidebar_layout(a, &l);
  return l.new_arrow.w && box_contains(l.new_arrow, x, y);
}

static const char *agent_tip(const SidebarLayout *l, int pane, char *buf, size_t cap) {
  for (int i = 0; i < l->row_count; i++) {
    if (l->rows[i].pane == pane) agent_row_tip(&l->rows[i], buf, cap);
  }
  return buf;
}

const char *sidebar_tip_at(App *a, int x, int y, Box *box, char *buf, size_t cap) {
  SidebarLayout l;
  sidebar_layout(a, &l);
  SidebarHit h = sidebar_hit(a, &l, x, y, box);
  const Strings *s = strings();
  buf[0] = 0;
  switch (h.part) {
    case SIDEBAR_TOGGLE: return l.collapsed ? s->expand : s->collapse;
    case SIDEBAR_NEW: {
      char folder[256];
      new_agent_folder_name(a, folder, sizeof folder);
      format_new_agent_in(buf, cap, folder[0] ? folder : "~");
      return buf;
    }
    case SIDEBAR_NEW_MENU: return s->choose_agent_folder;
    case SIDEBAR_AGENT: return agent_tip(&l, h.index, buf, cap);
    case SIDEBAR_GROUP: return a->agents.groups.groups[h.index].cwd;
    case SIDEBAR_SESSION: {
      const RecentSession *session = &a->agents.shown[h.index];
      snprintf(buf, cap, "%s\n%s\n%s", session->title, session->cwd, s->delete_hint);
      return buf;
    }
    case SIDEBAR_OFFER_CLOSE:
    case SIDEBAR_PLUGIN_CLOSE: return s->hide;
    default: return NULL;
  }
}
