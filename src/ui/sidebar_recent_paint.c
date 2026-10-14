// Drawing the recent sessions (Heron's recent-sessions.css,
// recent-group-head.css and recent-session-row.css): a 12 px head, 14 px
// semibold folder heads with a count chip, 13 px rows, the delete question
// over a row, and 40 px faded edges counting what is out of sight.
#include <stdio.h>
#include <string.h>

#include "app/paths.h"
#include "app/strings.h"
#include "app/theme.h"
#include "base/clock.h"
#include "ui/sidebar_recent.h"
#include "ui/widgets.h"

static void paint_head(App *a, Box h) {
  const Strings *s = strings();
  float x = h.x + 16 * a->scale, y = h.y + 10 * a->scale, line = (float)app_px(a, 16);
  float title_w = widget_spaced_text(a, a->fonts[FONT_UI_12_SEMI], s->recent_sessions, x, y, line, 0.72f * a->scale,
                                     THEME_FG_MUTED);
  float hint_x = x + title_w + 8 * a->scale, right = h.x + h.w - 16 * a->scale;
  ui_text_draw(a->canvas, a->fonts[FONT_UI_12], s->recent_hint, hint_x, y, right - hint_x, line, THEME_FG_MUTED,
               ALIGN_RIGHT);
}

static void paint_group(App *a, const RecentItem *it, bool hover, uint64_t now) {
  const SessionGroup *g = &a->agents.groups.groups[it->index];
  Box b = it->box;
  float k = a->scale, x = b.x + 4 * k, right = b.x + b.w - 8 * k, y = b.y + 6 * k, h = (float)app_px(a, 19);
  if (hover) canvas_fill_round(a->canvas, b.x, b.y, b.w, b.h, 8 * k, THEME_HOVER);
  ui_text_draw(a->canvas, a->fonts[FONT_UI_14], recent_folded(a, g->cwd) ? "▸" : "▾", x, y, 14 * k, h, THEME_FG_DIM,
               ALIGN_CENTER);
  x += 20 * k;
  char text[1024];
  format_time_ago(text, sizeof text, g->modified, now);
  float when_w = ui_text_width(a->header_font, text) + 1;
  ui_text_draw(a->canvas, a->header_font, text, right - when_w, y + k, when_w, h, THEME_FG_MUTED, ALIGN_LEFT);
  char count[16];
  snprintf(count, sizeof count, "%d", g->count);
  UiFont *mono = a->fonts[FONT_MONO_11];
  float chip_w = ui_text_width(mono, count) + 12 * k, chip_h = (float)app_px(a, 15);
  folder_label(g->cwd, text, sizeof text);
  float name_w = ui_text_width(a->fonts[FONT_UI_14_SEMI], text) + 1, room = right - when_w - 12 * k - chip_w - x;
  if (name_w > room) name_w = room;
  ui_text_draw(a->canvas, a->fonts[FONT_UI_14_SEMI], text[0] ? text : g->cwd, x, y, name_w, h, THEME_FG_SOFT,
               ALIGN_LEFT);
  float cx = x + name_w + 6 * k, cy = y + (h - chip_h) / 2 + k;
  canvas_fill_round(a->canvas, cx, cy, chip_w, chip_h, 8 * k, THEME_CONTROL);
  ui_text_draw(a->canvas, mono, count, cx, cy, chip_w, chip_h, THEME_FG_MUTED, ALIGN_CENTER);
}

static void paint_ask(App *a, Box b) {
  float k = a->scale, r = 8 * k;
  const Strings *s = strings();
  canvas_fill_round(a->canvas, b.x, b.y, b.w, b.h, r, THEME_RAISED);
  canvas_stroke_round(a->canvas, b.x, b.y, b.w, b.h, r, 1, THEME_AGENT_BAD);
  Box yes, no;
  recent_ask_buttons(a, b, &yes, &no);
  const char *text = a->sidebar.note ? a->sidebar.note : s->delete_ask;
  float right = a->sidebar.note ? b.x + b.w - 4 * k : yes.x - 6 * k;
  ui_text_draw(a->canvas, a->fonts[FONT_UI_12], text, b.x + 9 * k, b.y, right - b.x - 9 * k, b.h, THEME_FG_SOFT,
               ALIGN_LEFT);
  if (a->sidebar.note) return;
  canvas_fill_round(a->canvas, yes.x, yes.y, yes.w, yes.h, 6 * k, THEME_AGENT_BAD);
  ui_text_draw(a->canvas, a->fonts[FONT_UI_12_SEMI], s->delete_yes, yes.x, yes.y, yes.w, yes.h, THEME_BG_APP,
               ALIGN_CENTER);
  canvas_fill_round(a->canvas, no.x, no.y, no.w, no.h, 6 * k, THEME_CONTROL);
  canvas_stroke_round(a->canvas, no.x, no.y, no.w, no.h, 6 * k, 1, THEME_BORDER_STRONG);
  ui_text_draw(a->canvas, a->fonts[FONT_UI_12], s->delete_no, no.x, no.y, no.w, no.h, THEME_FG_SOFT, ALIGN_CENTER);
}

static void paint_row(App *a, const RecentItem *it, bool hover, uint64_t now) {
  const RecentSession *session = &a->agents.shown[it->index];
  Box b = it->box;
  float k = a->scale, x = b.x + 9 * k, right = b.x + b.w - 9 * k;
  if (strcmp(a->sidebar.asking, session->id) == 0) {
    paint_ask(a, b);
    return;
  }
  if (hover) {
    canvas_fill_round(a->canvas, b.x, b.y, b.w, b.h, 8 * k, THEME_HOVER);
    canvas_stroke_round(a->canvas, b.x, b.y, b.w, b.h, 8 * k, 1, THEME_BORDER_STRONG);
  }
  char when[64];
  if (hover) snprintf(when, sizeof when, "%s ↵", strings()->resume);
  else format_time_ago(when, sizeof when, session->modified, now);
  float when_w = ui_text_width(a->header_font, when) + 1;
  uint32_t when_color = hover ? THEME_AGENT_WORK : THEME_FG_MUTED;
  ui_text_draw(a->canvas, a->header_font, when, right - when_w, b.y + k, when_w, b.h, when_color, ALIGN_LEFT);
  float title_w = right - when_w - 8 * k - x;
  uint32_t title_color = hover ? THEME_FG_SOFT : THEME_FG_DIM;
  ui_text_draw(a->canvas, a->fonts[FONT_UI_13], session->title, x, b.y, title_w, b.h, title_color, ALIGN_LEFT);
}

static void paint_edges(App *a, const RecentList *l) {
  float k = a->scale, h = 40 * k;
  Box v = l->view;
  char text[64];
  if (l->above) {
    canvas_fill_fade(a->canvas, v.x, v.y, v.w, h, THEME_BAR_BOTTOM, true, 0.35f);
    format_more(text, sizeof text, l->above, true);
    ui_text_draw(a->canvas, a->header_font, text, v.x, v.y + 2 * k, v.w, 16 * k, THEME_FG_DIM, ALIGN_CENTER);
  }
  if (l->below) {
    canvas_fill_fade(a->canvas, v.x, v.y + v.h - h, v.w, h, THEME_BAR_BOTTOM, false, 0.35f);
    format_more(text, sizeof text, l->below, false);
    ui_text_draw(a->canvas, a->header_font, text, v.x, v.y + v.h - 22 * k, v.w, 16 * k, THEME_FG_DIM, ALIGN_CENTER);
  }
}

void recent_list_paint(App *a, const RecentList *l, Box section) {
  canvas_fill_rect(a->canvas, section.x, section.y, section.w, 1, THEME_BORDER_SOFT);
  paint_head(a, l->head);
  uint64_t now = clock_now_ms();
  SidebarHit hover = a->sidebar.hover;
  Box v = l->view;
  canvas_push_clip(a->canvas, v.x, v.y, v.w, v.h);
  for (int i = 0; i < l->count; i++) {
    const RecentItem *it = &l->items[i];
    if (it->box.y + it->box.h < v.y || it->box.y > v.y + v.h) continue;
    SidebarPart part = it->group ? SIDEBAR_GROUP : SIDEBAR_SESSION;
    bool lit = hover.part == part && hover.index == it->index;
    if (it->group) paint_group(a, it, lit, now);
    else paint_row(a, it, lit, now);
  }
  paint_edges(a, l);
  canvas_pop_clip(a->canvas);
}
