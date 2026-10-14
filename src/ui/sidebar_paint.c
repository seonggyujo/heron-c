// Drawing the sidebar (Heron's agent-sidebar.css and new-agent.css): the
// bottom-bar background with its right border, the 12 px spaced title and
// the ‹ › button, the "New agent" split button, the agent rows, the empty
// line, the recent sessions and the note.
#include "app/strings.h"
#include "app/theme.h"
#include "ui/agent_row.h"
#include "ui/new_agent.h"
#include "ui/sidebar.h"
#include "ui/sidebar_hints.h"
#include "ui/sidebar_layout.h"
#include "ui/sidebar_recent.h"
#include "ui/widgets.h"

static bool lit(const App *a, SidebarPart part) { return a->sidebar.hover.part == part; }

static void paint_head(App *a, const SidebarLayout *l) {
  const Strings *s = strings();
  if (!l->collapsed) {
    widget_spaced_text(a, a->fonts[FONT_UI_12_SEMI], s->agents, l->title.x, l->title.y, l->title.h,
                       0.72f * a->scale, THEME_FG_MUTED);
  }
  Box t = l->toggle;
  bool hover = lit(a, SIDEBAR_TOGGLE);
  if (hover) canvas_fill_round(a->canvas, t.x, t.y, t.w, t.h, 6 * a->scale, THEME_HOVER);
  ui_text_draw(a->canvas, a->fonts[FONT_UI_16], l->collapsed ? "›" : "‹", t.x, t.y - a->scale, t.w, t.h,
               hover ? THEME_FG : THEME_FG_DIM, ALIGN_CENTER);
}

// "+ New agent   folder": its side of the group, filled up to the ▾.
static void paint_new_main(App *a, const SidebarLayout *l, Box group, bool hover) {
  float k = a->scale;
  Box m = l->new_main;
  canvas_push_clip(a->canvas, m.x, m.y, l->new_arrow.x - m.x, m.h);
  canvas_fill_round(a->canvas, group.x, group.y, group.w, group.h, 9 * k, hover ? THEME_HOVER : THEME_CONTROL);
  canvas_pop_clip(a->canvas);
  UiFont *plus_font = a->fonts[FONT_UI_16], *label_font = a->fonts[FONT_UI_14_SEMI];
  float x = m.x + 12 * k, plus_w = ui_text_width(plus_font, "+") + 1;
  ui_text_draw(a->canvas, plus_font, "+", x, m.y - k, plus_w, m.h, THEME_AGENT_WORK, ALIGN_LEFT);
  x += plus_w + 8 * k;
  const char *label = strings()->new_agent;
  float label_w = ui_text_width(label_font, label) + 1;
  ui_text_draw(a->canvas, label_font, label, x, m.y, label_w, m.h, hover ? THEME_FG : THEME_FG_SOFT, ALIGN_LEFT);
  x += label_w + 8 * k;
  char folder[256];
  new_agent_folder_name(a, folder, sizeof folder);
  float right = l->new_arrow.x - 12 * k;
  if (folder[0] && right > x) {
    ui_text_draw(a->canvas, a->fonts[FONT_MONO_11], folder, x, m.y + k, right - x, m.h, THEME_FG_MUTED, ALIGN_RIGHT);
  }
}

// The ▾ of the folder menu, its side of the group behind a soft line.
static void paint_new_arrow(App *a, const SidebarLayout *l, Box group, bool hover) {
  Box w = l->new_arrow;
  canvas_push_clip(a->canvas, w.x, w.y, w.w, w.h);
  canvas_fill_round(a->canvas, group.x, group.y, group.w, group.h, 9 * a->scale, hover ? THEME_HOVER : THEME_CONTROL);
  canvas_pop_clip(a->canvas);
  canvas_fill_rect(a->canvas, w.x, w.y + 1, 1, w.h - 2, THEME_BORDER_SOFT);
  ui_text_draw(a->canvas, a->small_font, "▾", w.x, w.y, w.w, w.h, hover ? THEME_FG : THEME_FG_MUTED, ALIGN_CENTER);
}

static void paint_new_agent(App *a, const SidebarLayout *l) {
  float k = a->scale, r = 9 * k;
  Box m = l->new_main;
  bool main_lit = lit(a, SIDEBAR_NEW) || a->sidebar.pressed.part == SIDEBAR_NEW;
  if (l->collapsed) {
    canvas_fill_round(a->canvas, m.x, m.y, m.w, m.h, r, main_lit ? THEME_HOVER : THEME_CONTROL);
    canvas_stroke_round(a->canvas, m.x, m.y, m.w, m.h, r, 1, THEME_BORDER_STRONG);
    ui_text_draw(a->canvas, a->fonts[FONT_UI_16], "+", m.x, m.y - k, m.w, m.h, THEME_AGENT_WORK, ALIGN_CENTER);
    return;
  }
  Box group = {m.x, m.y, l->new_arrow.x + l->new_arrow.w - m.x, m.h};
  paint_new_main(a, l, group, main_lit);
  paint_new_arrow(a, l, group, lit(a, SIDEBAR_NEW_MENU) || a->popover == POPOVER_AGENT_FOLDERS);
  canvas_stroke_round(a->canvas, group.x, group.y, group.w, group.h, r, 1, THEME_BORDER_STRONG);
}

static void paint_rows(App *a, const SidebarLayout *l) {
  Box v = l->list;
  if (!v.h) return;
  canvas_push_clip(a->canvas, v.x, v.y, v.w, v.h);
  SidebarHit hover = a->sidebar.hover;
  for (int i = 0; i < l->row_count; i++) {
    Box b = l->row_boxes[i];
    if (b.y + b.h < v.y || b.y > v.y + v.h) continue;
    bool lit_row = hover.part == SIDEBAR_AGENT && hover.index == l->rows[i].pane;
    agent_row_paint(a, &l->rows[i], b, l->collapsed, lit_row);
  }
  canvas_pop_clip(a->canvas);
}

void sidebar_paint(App *a) {
  SidebarLayout l;
  sidebar_layout(a, &l);
  Box f = l.frame;
  canvas_push_clip(a->canvas, f.x, f.y, f.w, f.h);
  canvas_fill_rect(a->canvas, f.x, f.y, f.w, f.h, THEME_BAR_BOTTOM);
  canvas_fill_rect(a->canvas, f.x + f.w - 1, f.y, 1, f.h, THEME_BORDER);
  paint_head(a, &l);
  paint_new_agent(a, &l);
  paint_rows(a, &l);
  if (l.empty.h) {
    float k = a->scale;
    ui_text_draw(a->canvas, a->fonts[FONT_UI_13], strings()->no_agents, l.empty.x + 16 * k, l.empty.y + 2 * k,
                 l.empty.w - 32 * k, 17 * k, THEME_FG_MUTED, ALIGN_LEFT);
  }
  if (l.recent.h) {
    RecentList recent;
    recent_list_layout(a, l.recent, &recent);
    recent_list_paint(a, &recent, l.recent);
  }
  if (l.foot.h) {
    canvas_fill_rect(a->canvas, l.foot.x, l.foot.y, l.foot.w, 1, THEME_BORDER_SOFT);
    HintsLayout hints;
    int pad = app_px(a, 14);
    sidebar_hints_layout(a, l.foot.x + pad, l.foot.y + 1 + app_px(a, 12), l.foot.w - 2 * pad, &hints);
    sidebar_hints_paint(a, &hints);
  }
  canvas_pop_clip(a->canvas);
}
