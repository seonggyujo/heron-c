// Drawing the usage bar (Heron's usage-bar.css): each meter a 12 px head
// over a 22 px bold mono figure and a 6 px track; each chip a 13 px mono
// name and figure in a bordered box.
#include <stdio.h>

#include "agent/usage.h"
#include "app/strings.h"
#include "app/theme.h"
#include "base/clock.h"
#include "ui/usage_bar.h"
#include "ui/usage_layout.h"

static uint32_t figure_color(Fill fill, uint32_t ok) {
  return fill == FILL_BAD ? THEME_AGENT_BAD : fill == FILL_WARN ? THEME_AGENT_NEED : ok;
}

static void paint_meter(App *a, Box m, const Limit *limit, int kind) {
  Canvas *c = a->canvas;
  canvas_fill_rect(c, (float)(m.x + m.w - 1), (float)m.y, 1, (float)m.h, THEME_BORDER_SOFT);
  float x = m.x + 18 * a->scale, w = m.w - 36 * a->scale;
  float head_h = (float)app_px(a, 16), main_h = (float)app_px(a, 29);
  float y = m.y + (m.h - head_h - 5 * a->scale - main_h) / 2;
  const Strings *s = strings();
  UiFont *small = a->fonts[FONT_UI_12];
  ui_text_draw(c, small, kind ? s->limit_weekly : s->limit_five_hour, x, y, w, head_h, THEME_FG_MUTED, ALIGN_LEFT);
  if (limit->has_reset) {
    char left[32], text[96];
    format_reset_in(left, sizeof left, limit->resets_at - (double)clock_now_ms() / 1000);
    format_limit_resets(text, sizeof text, left);
    ui_text_draw(c, small, text, x, y, w, head_h, THEME_FG_MUTED, ALIGN_RIGHT);
  }
  y += head_h + 5 * a->scale;
  double pct = limit->used < 0 ? 0 : limit->used > 100 ? 100 : limit->used;
  Fill fill = usage_fill(pct);
  char used[16];
  snprintf(used, sizeof used, "%d%%", (int)(pct + 0.5));
  float used_w = 3.1f * 22 * a->scale;  // min-width: 3.1em
  ui_text_draw(c, a->fonts[FONT_MONO_22_BOLD], used, x, y, used_w, main_h, figure_color(fill, THEME_FG_SOFT),
               ALIGN_LEFT);
  float tx = x + used_w + 10 * a->scale, tw = x + w - tx, th = 6 * a->scale, ty = y + (main_h - th) / 2;
  canvas_fill_round(c, tx, ty, tw, th, th / 2, THEME_BORDER_SOFT);
  if (pct > 0) canvas_fill_round(c, tx, ty, (float)(tw * pct / 100), th, th / 2, figure_color(fill, THEME_AGENT_WORK));
}

static void paint_chip(App *a, Box b, const AgentRow *r) {
  Canvas *c = a->canvas;
  float radius = 8 * a->scale;
  if (a->chip_hover == r->pane) canvas_fill_round(c, b.x, b.y, b.w, b.h, radius, THEME_HOVER);
  canvas_stroke_round(c, b.x, b.y, b.w, b.h, radius, 1, THEME_BORDER);
  char name[300], used[16];
  int percent;
  usage_chip_texts(r, name, sizeof name, &percent);
  snprintf(used, sizeof used, "%d%%", percent);
  float x = b.x + 11 * a->scale, name_w = ui_text_width(a->big_font, name) + 1;
  ui_text_draw(c, a->big_font, name, x, b.y, name_w, b.h, THEME_FG_SOFT, ALIGN_LEFT);
  x += name_w + 7 * a->scale;
  uint32_t color = figure_color(usage_fill(percent), THEME_AGENT_WORK);
  ui_text_draw(c, a->fonts[FONT_MONO_13_BOLD], used, x, b.y, b.x + b.w - x, b.h, color, ALIGN_LEFT);
}

void usage_bar_paint(App *a) {
  if (!a->usage_shown) return;
  UsageLayout l;
  usage_layout(a, &l);
  Box bar = l.bar;
  canvas_fill_rect(a->canvas, (float)bar.x, (float)bar.y, (float)bar.w, (float)bar.h, THEME_BAR_BOTTOM);
  canvas_fill_rect(a->canvas, (float)bar.x, (float)bar.y, (float)bar.w, 1, THEME_BORDER);
  for (int i = 0; i < l.meter_count; i++) paint_meter(a, l.meters[i], l.limits[i], l.limit_kinds[i]);
  if (!l.chip_count) return;
  Box ctx = l.context;
  ui_text_draw(a->canvas, a->fonts[FONT_UI_12], strings()->context, l.label.x, l.label.y, l.label.w, l.label.h,
               THEME_FG_MUTED, ALIGN_LEFT);
  canvas_push_clip(a->canvas, (float)ctx.x, (float)ctx.y, (float)ctx.w - 18 * a->scale, (float)ctx.h);
  for (int i = 0; i < l.chip_count; i++) paint_chip(a, l.chips[i], &l.rows[i]);
  canvas_pop_clip(a->canvas);
}
