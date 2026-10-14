// Laying out, drawing and hit testing the title bar.
#include "ui/titlebar.h"

#include <math.h>
#include <stdio.h>

#include "app/strings.h"
#include "app/theme.h"
#include "ui/titlebar_parts.h"
#include "ui/widgets.h"

void titlebar_label(App *a, char *out, size_t cap) {
  snprintf(out, cap, "+  %s", strings()->new_terminal);
}

int titlebar_pill_width(App *a) {
  float widest = 0;
  for (int k = 0; k < SHELL_COUNT; k++) {
    float w = ui_text_width(a->header_font, shell_name((ShellKind)k));
    if (w > widest) widest = w;
  }
  int width = (int)ceilf(widest) + app_px(a, 14);
  return width > app_px(a, 80) ? width : app_px(a, 80);
}

void titlebar_count_text(App *a, char *out, size_t cap) {
  format_running(out, cap, panes_running(&a->panes), a->panes.shown.count);
}

void titlebar_layout(App *a, TitleLayout *t) {
  RECT rc;
  GetClientRect(a->hwnd, &rc);
  int h = a->m.titlebar_h, light = app_px(a, 16), gap = app_px(a, 10);
  int x = rc.right - app_px(a, 14);
  t->close = (Box){x - light, (h - light) / 2, light, light};
  t->max = (Box){t->close.x - gap - light, t->close.y, light, light};
  t->min = (Box){t->max.x - gap - light, t->close.y, light, light};

  char label[96];
  titlebar_label(a, label, sizeof label);
  float text_w = ui_text_width(a->ui_font, label) + app_px(a, 8) + titlebar_pill_width(a);
  int bh = app_px(a, 26), by = (h - bh) / 2, menu_w = app_px(a, 24);
  x = t->min.x - app_px(a, 14);
  t->menu_button = (Box){x - menu_w, by, menu_w, bh};
  int new_w = (int)ceilf(text_w) + app_px(a, 12 + 5);
  t->new_button = (Box){t->menu_button.x - app_px(a, 2) - new_w, by, new_w, bh};
  t->settings = (Box){t->new_button.x - app_px(a, 8 + 30), by, app_px(a, 30), bh};
  char count[64];
  titlebar_count_text(a, count, sizeof count);
  int count_w = (int)ceilf(ui_text_width(a->header_font, count));
  t->count = (Box){t->settings.x - app_px(a, 8 + 6) - count_w, 0, count_w, h};
  t->tidy = (Box){t->count.x - app_px(a, 8 + 30), by, app_px(a, 30), bh};
}

HitTarget titlebar_hit(App *a, int x, int y) {
  if (y >= a->m.titlebar_h) return HIT_NONE;
  TitleLayout t;
  titlebar_layout(a, &t);
  if (box_contains(t.new_button, x, y)) return HIT_NEW;
  if (box_contains(t.menu_button, x, y)) return HIT_SHELL_MENU;
  if (box_contains(t.settings, x, y)) return HIT_SETTINGS;
  if (a->panes.arrangement.manual && box_contains(t.tidy, x, y)) return HIT_TIDY;
  if (box_contains(t.min, x, y)) return HIT_MIN;
  if (box_contains(t.max, x, y)) return HIT_MAX;
  if (box_contains(t.close, x, y)) return HIT_CLOSE;
  return HIT_NONE;
}

// "● HERON", letters spaced apart like the original's letter-spacing.
static void paint_brand(App *a) {
  float left = (float)app_px(a, 16), h = (float)a->m.titlebar_h;
  float dot_w = ui_text_width(a->brand_font, "\xe2\x97\x8f");
  ui_text_draw(a->canvas, a->brand_font, "\xe2\x97\x8f", left, 0, dot_w + 1, h, THEME_FG, ALIGN_LEFT);
  widget_spaced_text(a, a->brand_font, "HERON", left + dot_w + app_px(a, 7), 0, h, 1.7f * a->scale, THEME_FG);
}

// Dimmer while nothing runs, as Heron's .tb-count.idle.
static void paint_count(App *a, Box b) {
  char text[64];
  titlebar_count_text(a, text, sizeof text);
  uint32_t color = THEME_FG_DIM;
  if (panes_running(&a->panes) == 0) color = ((uint32_t)((color >> 24) * 0.6f) << 24) | (color & 0xFFFFFF);
  ui_text_draw(a->canvas, a->header_font, text, (float)b.x, 0, (float)b.w + 1, (float)b.h, color, ALIGN_LEFT);
}

void titlebar_paint(App *a) {
  RECT rc;
  GetClientRect(a->hwnd, &rc);
  float w = (float)rc.right, h = (float)a->m.titlebar_h;
  canvas_fill_rect(a->canvas, 0, 0, w, h, THEME_BAR);
  canvas_fill_rect(a->canvas, 0, h - 1, w, 1, THEME_BORDER);
  TitleLayout t;
  titlebar_layout(a, &t);
  paint_brand(a);
  if (a->panes.arrangement.manual) titlebar_paint_tidy(a, &t);
  paint_count(a, t.count);
  titlebar_paint_settings(a, &t);
  titlebar_paint_split_button(a, &t);
  titlebar_paint_lights(a, &t);
}
