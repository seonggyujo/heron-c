// The title bar's buttons: back to the automatic grid, settings gear, new
// terminal split button, and the window lights.
#include <math.h>

#include "app/theme.h"
#include "ui/titlebar_parts.h"
#include "ui/widgets.h"

static WidgetState state_of(const App *a, HitTarget hit, bool open) {
  if (a->pressed == hit) return WIDGET_PRESSED;
  return open ? WIDGET_LIT : a->hover == hit ? WIDGET_HOVER : WIDGET_NORMAL;
}

// A gear: a small ring and eight spokes (Heron's 16-unit icon).
static void paint_gear(App *a, Box b, uint32_t color) {
  float k = a->scale * 15.0f / 16.0f, cx = b.x + b.w / 2.0f, cy = b.y + b.h / 2.0f, line = 1.3f * a->scale;
  canvas_stroke_circle(a->canvas, cx, cy, 2.2f * k + line / 2, line, color);
  for (int i = 0; i < 8; i++) {
    float angle = i * 3.14159265f / 4, c = cosf(angle), s = sinf(angle);
    float inner = 4.5f * k, outer = 6.5f * k;
    canvas_line(a->canvas, cx + c * inner, cy + s * inner, cx + c * outer, cy + s * outer, line, color);
  }
}

// Four rounded squares (Heron's 14-unit tidy icon).
void titlebar_paint_tidy(App *a, const TitleLayout *t) {
  WidgetState s = state_of(a, HIT_TIDY, false);
  widget_button(a, t->tidy, NULL, NULL, s);
  float k = a->scale, size = 14 * k, line = 1.2f * k;
  float left = t->tidy.x + (t->tidy.w - size) / 2, top = t->tidy.y + (t->tidy.h - size) / 2;
  uint32_t color = s == WIDGET_NORMAL ? THEME_FG_DIM : THEME_FG;
  const float at[2] = {1.5f, 8.0f};
  for (int i = 0; i < 4; i++) {
    float x = left + at[i % 2] * k, y = top + at[i / 2] * k;
    canvas_stroke_round(a->canvas, x - line / 2, y - line / 2, 4.5f * k + line, 4.5f * k + line, 1 * k + line / 2,
                        line, color);
  }
}

void titlebar_paint_settings(App *a, const TitleLayout *t) {
  WidgetState s = state_of(a, HIT_SETTINGS, a->popover == POPOVER_SETTINGS);
  widget_button(a, t->settings, NULL, NULL, s);
  paint_gear(a, t->settings, s == WIDGET_NORMAL ? THEME_FG_DIM : THEME_FG);
}

void titlebar_paint_split_button(App *a, const TitleLayout *t) {
  float k = a->scale;
  Box b = t->new_button;
  widget_button(a, b, NULL, NULL, state_of(a, HIT_NEW, false));
  char label[96];
  titlebar_label(a, label, sizeof label);
  float x = b.x + 12 * k, label_w = ui_text_width(a->ui_font, label);
  ui_text_draw(a->canvas, a->ui_font, label, x, (float)b.y, label_w + 1, (float)b.h, THEME_FG_SOFT,
               ALIGN_LEFT);
  x += label_w + 8 * k;
  float pill_w = (float)titlebar_pill_width(a), pill_h = b.h - 8 * k;
  canvas_fill_round(a->canvas, x, b.y + 4 * k, pill_w, pill_h, 5 * k, THEME_INSET);
  ui_text_draw(a->canvas, a->header_font, shell_name(a->shell), x, b.y + 4 * k, pill_w, pill_h, THEME_FG_DIM,
               ALIGN_CENTER);

  Box m = t->menu_button;
  widget_button(a, m, NULL, NULL, state_of(a, HIT_SHELL_MENU, a->popover == POPOVER_SHELLS));
  float cx = m.x + m.w / 2.0f, cy = m.y + m.h / 2.0f, d = 3.5f * k;
  canvas_line(a->canvas, cx - d, cy - d / 2, cx, cy + d / 2, 1.4f * k, THEME_FG_DIM);
  canvas_line(a->canvas, cx, cy + d / 2, cx + d, cy - d / 2, 1.4f * k, THEME_FG_DIM);
}

static bool lights_hovered(const App *a) {
  return a->hover == HIT_MIN || a->hover == HIT_MAX || a->hover == HIT_CLOSE;
}

static uint32_t darker(uint32_t argb, float k) {
  uint32_t r = (uint32_t)(((argb >> 16) & 0xFF) * k), g = (uint32_t)(((argb >> 8) & 0xFF) * k),
           b = (uint32_t)((argb & 0xFF) * k);
  return (argb & 0xFF000000u) | (r << 16) | (g << 8) | b;
}

// Heron's 10-unit glyphs drawn 12 px wide: a cross, a dash, and the two
// corner triangles of the macOS zoom button. They show on all three lights
// while any of them is hovered, as on macOS; a pressed light darkens.
static void paint_light(App *a, Box b, uint32_t color, HitTarget hit) {
  float k = a->scale, r = b.w / 2.0f, cx = b.x + r, cy = b.y + r, u = 1.2f * k, line = 1.2f * u;
  bool lit = a->active || lights_hovered(a);
  if (a->pressed == hit) color = darker(color, 0.8f);
  canvas_fill_circle(a->canvas, cx, cy, r, lit ? color : THEME_LIGHT_OFF);
  canvas_stroke_circle(a->canvas, cx, cy, r, 0.5f * k, lit ? THEME_LIGHT_RING : THEME_LIGHT_OFF_RING);
  if (!lights_hovered(a)) return;
  uint32_t glyph = THEME_LIGHT_GLYPH;
  if (hit == HIT_MIN) canvas_line(a->canvas, cx - 2.5f * u, cy, cx + 2.5f * u, cy, line, glyph);
  if (hit == HIT_CLOSE) {
    float d = 2 * u;
    canvas_line(a->canvas, cx - d, cy - d, cx + d, cy + d, line, glyph);
    canvas_line(a->canvas, cx + d, cy - d, cx - d, cy + d, line, glyph);
  }
  if (hit == HIT_MAX) {
    float outer = 2.4f * u, inner = 1.2f * u;
    const float top[6] = {cx - outer, cy - outer, cx + inner, cy - outer, cx - outer, cy + inner};
    const float bottom[6] = {cx + outer, cy + outer, cx - inner, cy + outer, cx + outer, cy - inner};
    canvas_fill_triangle(a->canvas, top, glyph);
    canvas_fill_triangle(a->canvas, bottom, glyph);
  }
}

void titlebar_paint_lights(App *a, const TitleLayout *t) {
  paint_light(a, t->min, THEME_LIGHT_MIN, HIT_MIN);
  paint_light(a, t->max, THEME_LIGHT_MAX, HIT_MAX);
  paint_light(a, t->close, THEME_LIGHT_CLOSE, HIT_CLOSE);
}
