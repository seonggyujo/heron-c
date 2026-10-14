// Laying out the sidebar's column of parts.
#include "ui/sidebar_layout.h"

#include "ui/agent_row.h"
#include "ui/sidebar.h"
#include "ui/sidebar_hints.h"
#include "ui/usage_bar.h"

static int clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

static void lay_head(App *a, SidebarLayout *l, int w) {
  int top = l->frame.y, head = app_px(a, 38), t = app_px(a, 26);
  int x = l->collapsed ? (w - t) / 2 : w - app_px(a, 8) - t;
  l->toggle = (Box){x, top + (head - t) / 2, t, t};
  l->title = (Box){app_px(a, 16), top, l->toggle.x - app_px(a, 16), head};
  int pad = app_px(a, l->collapsed ? 6 : 10), h = app_px(a, 36), arrow = l->collapsed ? 0 : app_px(a, 28);
  int y = top + head;
  // The ▾ overlaps the main part by its 1 px border (margin-left: -1px).
  l->new_main = (Box){pad, y, w - 2 * pad - (arrow ? arrow - 1 : 0), h};
  l->new_arrow = (Box){l->new_main.x + l->new_main.w - 1, y, arrow, h};
}

// The rows' heights, and all of them with their 4 px gaps.
static int lay_rows(App *a, SidebarLayout *l, int *heights) {
  l->row_count = agent_rows(a, l->rows, AGENT_ROWS_MAX);
  agent_rows_by_urgency(l->rows, l->row_count);
  int total = 0;
  for (int i = 0; i < l->row_count; i++) {
    heights[i] = agent_row_height(a, &l->rows[i], l->collapsed);
    total += heights[i] + (i ? app_px(a, 4) : 0);
  }
  return total;
}

static void place_rows(App *a, SidebarLayout *l, const int *heights, int w) {
  l->scroll = clamp(a->sidebar.agents_scroll, 0, l->rows_h > l->list.h ? l->rows_h - l->list.h : 0);
  int y = l->list.y - l->scroll, pad = app_px(a, 10);
  for (int i = 0; i < l->row_count; i++) {
    l->row_boxes[i] = (Box){pad, y, w - 2 * pad, heights[i]};
    y += heights[i] + app_px(a, 4);
  }
}

void sidebar_layout(App *a, SidebarLayout *l) {
  RECT rc;
  GetClientRect(a->hwnd, &rc);
  int top = a->m.titlebar_h, bottom = rc.bottom - usage_bar_height(a);
  l->collapsed = a->sidebar.collapsed;
  l->frame = (Box){0, top, sidebar_width(a), bottom - top};
  int w = l->frame.w - 1;  // inside the right border
  lay_head(a, l, w);
  int heights[AGENT_ROWS_MAX];
  l->rows_h = lay_rows(a, l, heights);
  int y = l->new_main.y + l->new_main.h + app_px(a, 8);

  bool open = !l->collapsed, recent = open && a->agents.shown_count > 0;
  int empty_h = open && l->row_count == 0 ? app_px(a, 23) : 0;
  HintsLayout hints;
  int pad = app_px(a, 14);
  if (open) sidebar_hints_layout(a, pad, 0, w - 2 * pad, &hints);
  int foot_h = open && hints.h ? hints.h + app_px(a, 24) + 1 : 0;
  int room = bottom - y - empty_h - foot_h - (recent ? app_px(a, 10) : 0);
  int recent_h = 0;
  if (recent) {
    int least = app_px(a, 96);
    recent_h = room - l->rows_h > least ? room - l->rows_h : least;
  }
  l->list = (Box){0, y, w, clamp(room - recent_h, 0, l->rows_h)};
  place_rows(a, l, heights, w);
  y += l->list.h;
  l->empty = (Box){0, y, w, empty_h};
  y += empty_h + (recent ? app_px(a, 10) : 0);
  l->recent = (Box){0, y, w, recent_h};
  l->foot = (Box){0, y + recent_h, w, foot_h};
}
