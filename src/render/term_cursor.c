// The cursor (block, underline or bar; hollow when the pane is not focused)
// and the IME composition text, which takes the cursor's place while typing
// Korean.
#include "render/cell_style.h"
#include "render/glyph_run.h"
#include "render/term_cursor.h"
#include "vt/palette.h"
#include "vt/utf8_decoder.h"
#include "vt/wcwidth.h"

#define CURSOR_COLOR (0xFF000000u | PALETTE_CURSOR)
#define MAX_COMPOSITION 64

static void paint_composition(Canvas *c, const MonoFont *f, const char *text, float x, float y) {
  uint32_t cps[MAX_COMPOSITION];
  uint8_t widths[MAX_COMPOSITION];
  int n = 0, cells = 0;
  Utf8Decoder d = {0};
  for (const char *p = text; *p && n < MAX_COMPOSITION - 1; p++) {
    uint32_t out[2];
    int got = utf8_decoder_feed(&d, (uint8_t)*p, out);
    for (int i = 0; i < got && n < MAX_COMPOSITION; i++) {
      int w = vt_wcwidth(out[i]);
      if (w == 0) continue;
      cps[n] = out[i];
      widths[n++] = (uint8_t)w;
      cells += w;
    }
  }
  float width = (float)(cells * f->cell_w);
  canvas_fill_rect(c, x, y, width, (float)f->cell_h, cell_default_bg());
  glyph_run_draw(c, f, FACE_REGULAR, cps, widths, n, x, y + f->baseline, CURSOR_COLOR);
  canvas_fill_rect(c, x, y + f->cell_h - f->underline_h, width, f->underline_h, CURSOR_COLOR);
}

void term_paint_cursor(Canvas *c, const TermView *v) {
  if (term_view_offset(v->term) != 0) return;
  const MonoFont *f = v->font;
  int cx, cy;
  term_cursor(v->term, &cx, &cy);
  float x = v->x + cx * f->cell_w, y = v->y + cy * f->cell_h;
  if (v->composition && v->composition[0]) {
    paint_composition(c, f, v->composition, x, y);
    return;
  }
  TermModes modes = term_modes(v->term);
  if (!modes.cursor_visible) return;
  const Line *l = term_view_line(v->term, cy);
  Cell cell = cx < l->len ? l->cells[cx] : (Cell){0, COLOR_DEFAULT, COLOR_DEFAULT, 0, 0};
  float w = (float)(((cell.attr & ATTR_WIDE) ? 2 : 1) * f->cell_w), h = (float)f->cell_h;
  if (!v->focused) {
    canvas_stroke_round(c, x, y, w, h, 0, 1, CURSOR_COLOR);
  } else if (modes.cursor_style == 3 || modes.cursor_style == 4) {
    canvas_fill_rect(c, x, y + h - 2, w, 2, CURSOR_COLOR);
  } else if (modes.cursor_style == 5 || modes.cursor_style == 6) {
    canvas_fill_rect(c, x, y, 2, h, CURSOR_COLOR);
  } else {
    canvas_fill_rect(c, x, y, w, h, CURSOR_COLOR);
    if (cell.ch && cell.ch != ' ') {
      uint8_t width = (cell.attr & ATTR_WIDE) ? 2 : 1;
      CellStyle s = cell_style(&cell, false);
      glyph_run_draw(c, f, s.face, &cell.ch, &width, 1, x, y + f->baseline, cell_default_bg());
    }
  }
}
