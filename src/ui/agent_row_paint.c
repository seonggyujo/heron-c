// Drawing an agent row (Heron's agent-item.css, agent-marks.css and
// subagent-lines.css): a 24 px badge with a 7 px radius, 11 px from the
// body; 15 px semibold folder, 11 px mono model chip, 12 px mono time;
// 13 px state line with 11 px mono counts; 12 px subagent lines.
#include <stdio.h>

#include "agent/model_label.h"
#include "app/strings.h"
#include "app/theme.h"
#include "base/clock.h"
#include "ui/agent_row.h"

static uint32_t badge_color(AgentMark mark) {
  switch (mark) {
    case MARK_WORK: return THEME_AGENT_WORK;
    case MARK_NEED: return THEME_AGENT_NEED;
    case MARK_DONE: return THEME_AGENT_DONE;
    default: return THEME_FG_MUTED;
  }
}

static void paint_badge(App *a, const AgentRow *r, float x, float y) {
  float size = (float)app_px(a, 24);
  char n[12];
  snprintf(n, sizeof n, "%d", r->number);
  canvas_fill_round(a->canvas, x, y, size, size, 7 * a->scale, badge_color(agent_mark(r->agent->state)));
  ui_text_draw(a->canvas, a->fonts[FONT_MONO_13_BOLD], n, x, y, size, size, THEME_BG_APP, ALIGN_CENTER);
}

// Text drawn right-aligned against right; returns where it starts.
static float right_text(App *a, UiFont *f, const char *text, float right, float y, float h, uint32_t color) {
  float w = ui_text_width(f, text) + 1;
  ui_text_draw(a->canvas, f, text, right - w, y, w, h, color, ALIGN_RIGHT);
  return right - w;
}

static void paint_top(App *a, const AgentRow *r, float x, float y, float right, uint64_t now) {
  float h = (float)app_px(a, 20), gap = (float)app_px(a, 6), end = right;
  char text[160];
  if (r->agent->state == AGENT_WORKING) {
    format_uptime(text, sizeof text, now - r->agent->since);
    end = right_text(a, a->header_font, text, right, y + a->scale, h, THEME_FG_DIM) - gap;
  }
  agent_model_text(r->agent, text, sizeof text);
  if (text[0]) {
    UiFont *f = a->fonts[FONT_MONO_11];
    float w = ui_text_width(f, text) + 10 * a->scale, chip_h = (float)app_px(a, 15);
    float cx = end - w, cy = y + (h - chip_h) / 2 + a->scale;
    canvas_fill_round(a->canvas, cx, cy, w, chip_h, 5 * a->scale, THEME_INSET);
    ui_text_draw(a->canvas, f, text, cx, cy, w, chip_h, THEME_FG_MUTED, ALIGN_CENTER);
    end = cx - gap;
  }
  ui_text_draw(a->canvas, a->fonts[FONT_UI_15_SEMI], r->folder, x, y, end - x, h, THEME_FG_SOFT, ALIGN_LEFT);
}

static void paint_line(App *a, const AgentRow *r, float x, float y, float right) {
  float h = (float)app_px(a, 17), end = right;
  const GitChanges *changes = r->changes;
  if (changes && changes->files > 0) {
    UiFont *f = a->fonts[FONT_MONO_11];
    char added[24], removed[24];
    snprintf(added, sizeof added, "+%llu", (unsigned long long)changes->added);
    snprintf(removed, sizeof removed, "−%llu", (unsigned long long)changes->removed);
    end = right_text(a, f, removed, right, y + a->scale, h, THEME_AGENT_BAD) - 5 * a->scale;
    end = right_text(a, f, added, end, y + a->scale, h, THEME_AGENT_WORK) - 8 * a->scale;
  }
  char text[640];
  agent_line_text(r->agent, text, sizeof text);
  uint32_t color = agent_mark(r->agent->state) == MARK_NEED ? THEME_AGENT_NEED : THEME_FG_DIM;
  ui_text_draw(a->canvas, a->fonts[FONT_UI_13], text, x, y, end - x, h, color, ALIGN_LEFT);
}

static void paint_subs(App *a, const Agent *agent, float x, float y, float right, uint64_t now) {
  float h = (float)app_px(a, 16), step = (float)app_px(a, 17);
  int shown = agent->subagent_count < AGENT_SUBS_SHOWN ? agent->subagent_count : AGENT_SUBS_SHOWN;
  char text[96];
  for (int i = 0; i < shown; i++, y += step) {
    const Subagent *sub = &agent->subagents[i];
    format_uptime(text, sizeof text, now > sub->since ? now - sub->since : 0);
    float end = right_text(a, a->fonts[FONT_MONO_11], text, right, y + a->scale, h, THEME_FG_MUTED) - 6 * a->scale;
    snprintf(text, sizeof text, "↳ %s", sub->type);
    ui_text_draw(a->canvas, a->fonts[FONT_UI_12], text, x + 2 * a->scale, y, end - x, h, THEME_FG_MUTED, ALIGN_LEFT);
  }
  if (agent->subagent_count > AGENT_SUBS_SHOWN) {
    format_more_subagents(text, sizeof text, agent->subagent_count - AGENT_SUBS_SHOWN);
    ui_text_draw(a->canvas, a->fonts[FONT_UI_12], text, x + 2 * a->scale, y, right - x, h, THEME_FG_MUTED, ALIGN_LEFT);
  }
}

void agent_row_paint(App *a, const AgentRow *r, Box b, bool collapsed, bool hover) {
  float k = a->scale, radius = 10 * k, badge = (float)app_px(a, 24);
  if (hover) canvas_fill_round(a->canvas, b.x, b.y, b.w, b.h, radius, THEME_HOVER);
  if (agent_mark(r->agent->state) == MARK_NEED) {
    canvas_stroke_round(a->canvas, b.x, b.y, b.w, b.h, radius, 1, THEME_AGENT_NEED);
  }
  float by = b.y + (b.h - badge) / 2;
  if (collapsed) {
    paint_badge(a, r, b.x + (b.w - badge) / 2, by);
    return;
  }
  paint_badge(a, r, b.x + 11 * k, by);
  float x = b.x + (float)app_px(a, 46), right = (float)(b.x + b.w) - 11 * k, y = b.y + 11 * k;
  uint64_t now = clock_now_ms();
  paint_top(a, r, x, y, right, now);
  y += (float)app_px(a, 22);
  paint_line(a, r, x, y, right);
  y += (float)app_px(a, 19);
  if (r->agent->check == CHECK_MISMATCH) {
    char model[160], text[180];
    model_label(r->agent->actual[0] ? r->agent->actual : "?", model, sizeof model);
    snprintf(text, sizeof text, "⚠ %s", model);
    ui_text_draw(a->canvas, a->fonts[FONT_UI_13], text, x, y, right - x, (float)app_px(a, 17), THEME_AGENT_BAD,
                 ALIGN_LEFT);
    y += (float)app_px(a, 19);
  }
  if (r->agent->subagent_count) paint_subs(a, r->agent, x, y + 2 * k, right, now);
}
