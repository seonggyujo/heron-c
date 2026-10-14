// Drawing the shared controls (sizes and colors from Heron's controls.css
// and settings.css).
#include "ui/widgets.h"

#include <ctype.h>
#include <string.h>

#include "app/theme.h"

static void fill_box(App *a, Box b, float r, uint32_t color) {
  canvas_fill_round(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h, r, color);
}

static void stroke_box(App *a, Box b, float r, uint32_t color) {
  canvas_stroke_round(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h, r, 1, color);
}

void widget_panel(App *a, Box b, float radius) {
  for (int i = 1; i <= 4; i++) {
    float grow = i * 2.5f * a->scale, drop = i * 2.0f * a->scale;
    canvas_fill_round(a->canvas, b.x - grow, b.y - grow + drop, b.w + 2 * grow, b.h + 2 * grow, radius + grow,
                      0x18000000u);
  }
  fill_box(a, b, radius, THEME_RAISED);
  stroke_box(a, b, radius, THEME_BORDER);
}

void widget_button(App *a, Box b, UiFont *f, const char *text, WidgetState state) {
  float r = 7 * a->scale;
  bool lit = state == WIDGET_HOVER || state == WIDGET_LIT;
  uint32_t bg = state == WIDGET_PRESSED ? THEME_ACTIVE : lit ? THEME_HOVER : THEME_CONTROL;
  fill_box(a, b, r, bg);
  stroke_box(a, b, r, lit ? THEME_BORDER_STRONG : THEME_BORDER);
  if (text) {
    ui_text_draw(a->canvas, f, text, (float)b.x, (float)b.y, (float)b.w, (float)b.h,
                 lit ? THEME_FG : THEME_FG_DIM, ALIGN_CENTER);
  }
}

void widget_danger_button(App *a, Box b, UiFont *f, const char *text) {
  float r = 7 * a->scale;
  fill_box(a, b, r, THEME_ERROR_BG);
  stroke_box(a, b, r, THEME_ERROR_BORDER);
  ui_text_draw(a->canvas, f, text, (float)b.x, (float)b.y, (float)b.w, (float)b.h, THEME_ERROR_FG,
               ALIGN_CENTER);
}

void widget_switch(App *a, Box b, float on, bool hover) {
  float r = b.h / 2.0f, k = a->scale;
  bool lit = on >= 0.5f;
  fill_box(a, b, r, lit ? THEME_ACTIVE : THEME_INSET);
  stroke_box(a, b, r, lit ? THEME_FG_DIM : hover ? THEME_FG_MUTED : THEME_BORDER_STRONG);
  float knob = 12 * k, cx = b.x + 3 * k + knob / 2 + on * (b.w - knob - 6 * k);
  canvas_fill_circle(a->canvas, cx, b.y + b.h / 2.0f, knob / 2, lit ? THEME_FG : THEME_FG_MUTED);
}

void widget_swatch(App *a, Box b, uint32_t color, bool checked, bool hover) {
  float r = b.w / 2.0f, cx = b.x + r, cy = b.y + r;
  uint32_t alpha = checked ? 0xFF : hover ? 0xE6 : 0x8C;
  canvas_fill_circle(a->canvas, cx, cy, r, (alpha << 24) | (color & 0xFFFFFF));
  canvas_stroke_circle(a->canvas, cx, cy, r, 1, THEME_BORDER_STRONG);
  if (checked) canvas_stroke_circle(a->canvas, cx, cy, r + 3 * a->scale, 1, THEME_FG_DIM);
}

void widget_group(App *a, const Box *boxes, int n, UiFont *f, const char *const *texts, int lit,
                  int hover) {
  Box all = {boxes[0].x, boxes[0].y, boxes[n - 1].x + boxes[n - 1].w - boxes[0].x, boxes[0].h};
  float r = 7 * a->scale;
  fill_box(a, all, r, THEME_CONTROL);
  for (int i = 0; i < n; i++) {
    Box b = boxes[i];
    uint32_t bg = !texts[i] ? THEME_INSET : i == lit || i == hover ? THEME_HOVER : 0;
    if (bg) {
      canvas_push_clip(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h);
      fill_box(a, all, r, bg);
      canvas_pop_clip(a->canvas);
    }
    if (i > 0) canvas_fill_rect(a->canvas, (float)b.x, (float)b.y, 1, (float)b.h, THEME_BORDER);
  }
  stroke_box(a, all, r, lit >= 0 ? THEME_BORDER_STRONG : THEME_BORDER);
  for (int i = 0; i < n; i++) {
    if (!texts[i]) continue;
    uint32_t color = i == lit || i == hover ? THEME_FG : THEME_FG_DIM;
    ui_text_draw(a->canvas, f, texts[i], (float)boxes[i].x, (float)boxes[i].y, (float)boxes[i].w,
                 (float)boxes[i].h, color, ALIGN_CENTER);
  }
}

float widget_spaced_text(App *a, UiFont *f, const char *text, float x, float y, float h, float spacing,
                         uint32_t color) {
  float start = x;
  for (const char *p = text; *p;) {
    int len = 1;  // whole UTF-8 characters
    while ((p[len] & 0xC0) == 0x80) len++;
    char one[8] = {0};
    memcpy(one, p, (size_t)len);
    if (len == 1) one[0] = (char)toupper((unsigned char)one[0]);
    float w = ui_text_width(f, one);
    ui_text_draw(a->canvas, f, one, x, y, w + 1, h, color, ALIGN_LEFT);
    x += w + spacing;
    p += len;
  }
  return x - start;
}
