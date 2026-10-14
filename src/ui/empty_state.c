// The empty workspace (Heron's empty-state.css: a 96 px button with a 24 px
// radius, 14 px above a 13 px caption).
#include "ui/empty_state.h"

#include "app/strings.h"
#include "app/theme.h"

Box empty_state_button(const App *a) {
  Box w = app_workspace(a);
  int size = app_px(a, 96), caption = app_px(a, 14 + 18);
  Box b = {w.x + (w.w - size) / 2, w.y + (w.h - size - caption) / 2, size, size};
  return b;
}

bool empty_state_cursor_on(const App *a, unsigned long long now) {
  return !a->motion || (now / 550) % 2 == 0;
}

void empty_state_paint(App *a) {
  Box b = empty_state_button(a);
  bool hover = a->hover == HIT_EMPTY_NEW, pressed = a->pressed == HIT_EMPTY_NEW;
  float grow = pressed ? -0.03f : hover ? 0.04f : 0, d = b.w * grow / 2, r = 24 * a->scale;
  float x = b.x - d, y = b.y - d, w = b.w + 2 * d;
  canvas_fill_round(a->canvas, x, y, w, w, r, hover ? THEME_HOVER : THEME_RAISED);
  canvas_stroke_round(a->canvas, x, y, w, w, r, 1, hover ? THEME_BORDER_STRONG : THEME_BORDER);
  ui_text_draw(a->canvas, a->huge_font, "+", x, y - 3 * a->scale, w, w, THEME_FG, ALIGN_CENTER);

  const char *caption = strings()->empty_caption;
  float cw = ui_text_width(a->big_font, caption), under = ui_text_width(a->big_font, "_");
  float cx = b.x + b.w / 2.0f - (cw + under) / 2, cy = (float)(b.y + b.h + app_px(a, 14));
  ui_text_draw(a->canvas, a->big_font, caption, cx, cy, cw + 1, (float)app_px(a, 18), THEME_FG_DIM, ALIGN_LEFT);
  if (empty_state_cursor_on(a, GetTickCount64())) {
    ui_text_draw(a->canvas, a->big_font, "_", cx + cw, cy, under + 1, (float)app_px(a, 18), THEME_FG, ALIGN_LEFT);
  }
}
