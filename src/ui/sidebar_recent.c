// Laying out the recent sessions, folding folders and hit testing.
#include "ui/sidebar_recent.h"

#include <stdio.h>
#include <string.h>

#include "app/paths.h"
#include "app/prefs.h"
#include "app/strings.h"
#include "base/log.h"

// "|<key>|" for a folder.
static void fold_mark(const char *cwd, char *out, size_t cap) {
  char key[1024];
  folder_key(cwd, key, sizeof key);
  snprintf(out, cap, "|%s|", key);
}

bool recent_folded(const App *a, const char *cwd) {
  char mark[1100];
  fold_mark(cwd, mark, sizeof mark);
  return strstr(a->sidebar.folded, mark) != NULL;
}

void recent_toggle_fold(App *a, int group) {
  if (group < 0 || group >= a->agents.groups.count) return;
  char *folded = a->sidebar.folded, mark[1100];
  size_t cap = sizeof a->sidebar.folded;
  fold_mark(a->agents.groups.groups[group].cwd, mark, sizeof mark);
  char *at = strstr(folded, mark);
  if (at) {
    memmove(at, at + strlen(mark) - 1, strlen(at + strlen(mark) - 1) + 1);  // keep one '|'
    if (strcmp(folded, "|") == 0) folded[0] = 0;
  } else if (strlen(folded) + strlen(mark) < cap) {
    strcat(folded, folded[0] ? mark + 1 : mark);
  }
  prefs_set(PREF_FOLDED_GROUPS, folded);
  log_debug("sessions", "folded: %s", folded);
  app_redraw(a);
}

static void add(RecentList *l, bool group, int index, Box box) {
  if (l->count < RECENT_SESSIONS_MAX * 2) l->items[l->count++] = (RecentItem){group, index, box};
}

void recent_list_layout(App *a, Box s, RecentList *l) {
  l->head = (Box){s.x, s.y + 1, s.w, app_px(a, 32)};
  int top = l->head.y + l->head.h;
  l->view = (Box){s.x, top, s.w, s.y + s.h - top};
  l->count = 0;
  const SessionGroups *g = &a->agents.groups;
  int y = 0, pad = app_px(a, 10), gap = app_px(a, 2), indent = app_px(a, 14);
  int head_h = app_px(a, 29), row_h = app_px(a, 31);
  for (int k = 0; k < g->count; k++) {
    y += (l->count ? gap : 0) + app_px(a, 4);
    add(l, true, k, (Box){s.x + pad, y, s.w - 2 * pad, head_h});
    y += head_h;
    if (recent_folded(a, g->groups[k].cwd)) continue;
    for (int j = 0; j < g->groups[k].count; j++) {
      int index = (int)(g->order[g->groups[k].first + j] - a->agents.shown);
      y += gap;
      add(l, false, index, (Box){s.x + pad + indent, y, s.w - 2 * pad - indent, row_h});
      y += row_h;
    }
  }
  l->content_h = y + app_px(a, 8);
  int most = l->content_h > l->view.h ? l->content_h - l->view.h : 0, want = a->sidebar.recent_scroll;
  l->scroll = a->sidebar.recent_newest ? most : want < 0 ? 0 : want > most ? most : want;
  l->above = l->below = 0;
  for (int i = 0; i < l->count; i++) {
    Box *b = &l->items[i].box;
    b->y += l->view.y - l->scroll;
    if (l->items[i].group) continue;
    if (b->y < l->view.y - 1) l->above++;
    if (b->y + b->h > l->view.y + l->view.h + 1) l->below++;
  }
}

void recent_ask_buttons(App *a, Box row, Box *yes, Box *no) {
  UiFont *f = a->fonts[FONT_UI_12];
  const Strings *s = strings();
  int h = app_px(a, 22), y = row.y + (row.h - h) / 2, pad = app_px(a, 18);
  no->w = (int)ui_text_width(f, s->delete_no) + pad;
  yes->w = (int)ui_text_width(a->fonts[FONT_UI_12_SEMI], s->delete_yes) + pad;
  no->x = row.x + row.w - app_px(a, 5) - no->w;
  yes->x = no->x - app_px(a, 4) - yes->w;
  no->y = yes->y = y;
  no->h = yes->h = h;
}

SidebarHit recent_list_hit(App *a, const RecentList *l, int x, int y) {
  SidebarHit none = {SIDEBAR_NONE, 0};
  if (!box_contains(l->view, x, y)) return none;
  for (int i = 0; i < l->count; i++) {
    const RecentItem *it = &l->items[i];
    if (!box_contains(it->box, x, y)) continue;
    if (it->group) return (SidebarHit){SIDEBAR_GROUP, it->index};
    if (strcmp(a->sidebar.asking, a->agents.shown[it->index].id) != 0) return (SidebarHit){SIDEBAR_SESSION, it->index};
    Box yes, no;
    recent_ask_buttons(a, it->box, &yes, &no);
    bool buttons = !a->sidebar.note && !a->sidebar.deleting;
    if (buttons && box_contains(yes, x, y)) return (SidebarHit){SIDEBAR_ASK_YES, it->index};
    if (buttons && box_contains(no, x, y)) return (SidebarHit){SIDEBAR_ASK_NO, it->index};
    return (SidebarHit){SIDEBAR_ASK, it->index};
  }
  return none;
}
