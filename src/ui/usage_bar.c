// Showing the usage bar, laying it out, and its chips under the mouse.
#include "ui/usage_bar.h"

#include <math.h>
#include <stdio.h>

#include "app/strings.h"
#include "base/log.h"
#include "ui/usage_layout.h"

static bool wanted(const App *a) {
  const AgentBoard *b = &a->agents.board;
  if (b->has_limits && (b->limits.five_hour.known || b->limits.seven_day.known)) return true;
  for (int i = 0; i < b->count; i++) {
    if (b->entries[i].slot.present && b->entries[i].slot.agent.has_context) return true;
  }
  return false;
}

int usage_bar_height(const App *a) { return a->usage_shown ? app_px(a, 64) : 0; }

void usage_bar_sync(App *a) {
  bool shown = wanted(a);
  if (shown == a->usage_shown) return;
  a->usage_shown = shown;
  log_debug("usage", "usage bar %s", shown ? "shown" : "hidden");
  app_layout(a);
  app_redraw(a);
}

void usage_chip_texts(const AgentRow *r, char *name, size_t name_cap, int *percent) {
  snprintf(name, name_cap, "%d %s", r->number, r->folder);
  *percent = (int)lround(r->agent->context);
}

static void lay_chips(App *a, UsageLayout *l, int x) {
  AgentRow rows[AGENT_ROWS_MAX];
  int n = agent_rows(a, rows, AGENT_ROWS_MAX);
  l->chip_count = 0;
  for (int i = 0; i < n; i++) {
    if (rows[i].agent->has_context) l->rows[l->chip_count++] = rows[i];
  }
  Box bar = l->bar;
  l->context = (Box){x, bar.y, bar.x + bar.w - x, l->chip_count ? bar.h : 0};
  if (!l->chip_count) return;
  int label_w = (int)ui_text_width(a->fonts[FONT_UI_12], strings()->context) + 1;
  l->label = (Box){x + app_px(a, 18), bar.y, label_w, bar.h};
  int cx = l->label.x + label_w + app_px(a, 12), h = app_px(a, 29);
  for (int i = 0; i < l->chip_count; i++) {
    char name[300], used[16];
    int percent;
    usage_chip_texts(&l->rows[i], name, sizeof name, &percent);
    snprintf(used, sizeof used, "%d%%", percent);
    int w = (int)(ui_text_width(a->big_font, name) + ui_text_width(a->fonts[FONT_MONO_13_BOLD], used)) +
            app_px(a, 2 + 20 + 7) + 1;
    l->chips[i] = (Box){cx, bar.y + (bar.h - h) / 2, w, h};
    cx += w + app_px(a, 8);
  }
}

void usage_layout(App *a, UsageLayout *l) {
  RECT rc;
  GetClientRect(a->hwnd, &rc);
  int h = usage_bar_height(a);
  l->bar = (Box){0, rc.bottom - h, rc.right, h};
  l->meter_count = 0;
  const AgentBoard *b = &a->agents.board;
  const Limit *limits[2] = {&b->limits.five_hour, &b->limits.seven_day};
  int x = 0;
  for (int i = 0; i < 2; i++) {
    if (!b->has_limits || !limits[i]->known) continue;
    l->limits[l->meter_count] = limits[i];
    l->limit_kinds[l->meter_count] = i;
    l->meters[l->meter_count++] = (Box){x, l->bar.y, app_px(a, 240), h};
    x += app_px(a, 240);
  }
  lay_chips(a, l, x);
}

static int chip_at(const UsageLayout *l, int x, int y) {
  for (int i = 0; i < l->chip_count; i++) {
    if (box_contains(l->chips[i], x, y)) return i;
  }
  return -1;
}

bool usage_bar_mouse_move(App *a, int x, int y) {
  if (!a->usage_shown) return false;
  UsageLayout l;
  usage_layout(a, &l);
  int i = box_contains(l.bar, x, y) ? chip_at(&l, x, y) : -1;
  int pane = i >= 0 ? l.rows[i].pane : 0;
  if (pane != a->chip_hover) {
    a->chip_hover = pane;
    app_redraw(a);
  }
  return box_contains(l.bar, x, y);
}

bool usage_bar_click(App *a, int x, int y) {
  if (!a->usage_shown) return false;
  UsageLayout l;
  usage_layout(a, &l);
  if (!box_contains(l.bar, x, y)) return false;
  int i = chip_at(&l, x, y);
  Pane *p = i >= 0 ? panes_find(&a->panes, l.rows[i].pane) : NULL;
  if (p) app_focus_pane(a, p);
  return true;
}

const char *usage_bar_tip_at(App *a, int x, int y, Box *box, char *buf, size_t cap) {
  if (!a->usage_shown) return NULL;
  UsageLayout l;
  usage_layout(a, &l);
  int i = chip_at(&l, x, y);
  if (i < 0) return NULL;
  *box = l.chips[i];
  char name[300];
  int percent;
  usage_chip_texts(&l.rows[i], name, sizeof name, &percent);
  format_context_used(buf, cap, l.rows[i].folder, percent);
  return buf;
}
