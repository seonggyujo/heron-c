// Laying out and drawing the shell dropdown: a raised panel with a soft
// shadow, rows that light up under the mouse, and a check on the current
// shell.
#include <math.h>

#include "app/theme.h"
#include "ui/shell_menu.h"
#include "ui/shell_menu_layout.h"
#include "ui/titlebar.h"
#include "ui/widgets.h"

void shell_menu_layout(App *a, MenuLayout *m) {
  TitleLayout t;
  titlebar_layout(a, &t);
  int pad = app_px(a, 5), row_h = app_px(a, 30), text_left = app_px(a, 10 + 14 + 8);
  float widest = 0;
  m->count = 0;
  for (int k = 0; k < SHELL_COUNT; k++) {
    if (!shell_installed((ShellKind)k)) continue;
    m->kinds[m->count++] = (ShellKind)k;
    float w = ui_text_width(a->header_font, shell_name((ShellKind)k));
    if (w > widest) widest = w;
  }
  int width = (int)ceilf(widest) + text_left + app_px(a, 10) + 2 * pad;
  if (width < app_px(a, 170)) width = app_px(a, 170);
  int right = t.menu_button.x + t.menu_button.w;
  m->panel = (Box){right - width, t.menu_button.y + t.menu_button.h + app_px(a, 6), width,
                   2 * pad + m->count * row_h};
  for (int i = 0; i < m->count; i++) {
    m->rows[i] = (Box){m->panel.x + pad, m->panel.y + pad + i * row_h, width - 2 * pad, row_h};
  }
}

void shell_menu_paint(App *a) {
  MenuLayout m;
  shell_menu_layout(a, &m);
  Box p = m.panel;
  widget_panel(a, p, 9 * a->scale);
  for (int i = 0; i < m.count; i++) {
    Box row = m.rows[i];
    bool hover = i == a->popover_hover, current = m.kinds[i] == a->shell;
    if (hover) {
      canvas_fill_round(a->canvas, (float)row.x, (float)row.y, (float)row.w, (float)row.h, 6 * a->scale,
                        THEME_HOVER);
    }
    uint32_t color = hover ? THEME_FG : THEME_FG_DIM;
    float x = row.x + 10 * a->scale;
    if (current) {
      ui_text_draw(a->canvas, a->header_font, "\xe2\x9c\x93", x, (float)row.y, 14 * a->scale,
                   (float)row.h, THEME_FG, ALIGN_LEFT);  // ✓
    }
    x += 22 * a->scale;
    ui_text_draw(a->canvas, a->header_font, shell_name(m.kinds[i]), x, (float)row.y, row.x + row.w - x,
                 (float)row.h, color, ALIGN_LEFT);
  }
}
