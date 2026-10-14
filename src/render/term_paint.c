// Drawing the rows of a terminal: first the backgrounds that differ from the
// terminal's, then text in runs of the same color and face.
#include "render/term_paint.h"

#include "render/cell_style.h"
#include "render/glyph_run.h"
#include "render/term_cursor.h"

#define MAX_COLS 1024

static Cell cell_at(const Line *l, int x) {
  if (x < l->len) return l->cells[x];
  Cell empty = {0, COLOR_DEFAULT, COLOR_DEFAULT, 0, 0};
  return empty;
}

static CellStyle style_at(const TermView *v, const Line *l, uint64_t row, int x) {
  Cell cell = cell_at(l, x);
  return cell_style(&cell, selection_contains(v->selection, row, x));
}

static void paint_backgrounds(Canvas *c, const TermView *v, const Line *l, uint64_t row, float y) {
  const MonoFont *f = v->font;
  int cols = term_cols(v->term);
  int x = 0;
  while (x < cols) {
    uint32_t bg = style_at(v, l, row, x).bg;
    int start = x;
    while (x < cols && style_at(v, l, row, x).bg == bg) x++;
    if (bg != cell_default_bg()) {
      canvas_fill_rect(c, v->x + start * f->cell_w, y, (float)((x - start) * f->cell_w),
                       (float)f->cell_h, bg);
    }
  }
}

static bool same_ink(CellStyle a, CellStyle b) {
  return a.fg == b.fg && a.face == b.face && a.underline == b.underline && a.strike == b.strike;
}

static void paint_text(Canvas *c, const TermView *v, const Line *l, uint64_t row, float y) {
  const MonoFont *f = v->font;
  uint32_t cps[MAX_COLS];
  uint8_t widths[MAX_COLS];
  int cols = term_cols(v->term) < MAX_COLS ? term_cols(v->term) : MAX_COLS;
  int x = 0;
  while (x < cols) {
    CellStyle s = style_at(v, l, row, x);
    int start = x, n = 0;
    bool ink = false;
    while (x < cols && same_ink(style_at(v, l, row, x), s)) {
      // Wide characters step over their right half, so a half met here is
      // an orphan and draws as a space.
      Cell cell = cell_at(l, x);
      bool tail = cell.attr & ATTR_WIDE_TAIL;
      cps[n] = cell.ch && !tail ? cell.ch : ' ';
      widths[n] = (cell.attr & ATTR_WIDE) && x + 1 < cols ? 2 : 1;
      ink = ink || cps[n] != ' ';
      x += widths[n++];
    }
    float left = v->x + start * f->cell_w, width = (float)((x - start) * f->cell_w);
    if (ink) glyph_run_draw(c, f, s.face, cps, widths, n, left, y + f->baseline, s.fg);
    if (s.underline) canvas_fill_rect(c, left, y + f->underline_y, width, f->underline_h, s.fg);
    if (s.strike) canvas_fill_rect(c, left, y + f->strike_y, width, f->underline_h, s.fg);
  }
}

// The link under the mouse is underlined in the color of its first cell.
static void paint_link(Canvas *c, const TermView *v, const Line *l, uint64_t row, float y) {
  const TermLink *k = v->link;
  if (!k || row < k->row0 || row > k->row1) return;
  int c0 = row == k->row0 ? k->col0 : 0, c1 = row == k->row1 ? k->col1 : term_cols(v->term) - 1;
  const MonoFont *f = v->font;
  CellStyle s = style_at(v, l, row, c0);
  canvas_fill_rect(c, v->x + c0 * f->cell_w, y + f->underline_y, (float)((c1 - c0 + 1) * f->cell_w),
                   f->underline_h, s.fg);
}

void term_paint(Canvas *c, const TermView *v) {
  int rows = term_rows(v->term);
  for (int r = 0; r < rows; r++) {
    const Line *l = term_view_line(v->term, r);
    uint64_t row = term_view_row_id(v->term, r);
    float y = v->y + (float)(r * v->font->cell_h);
    paint_backgrounds(c, v, l, row, y);
    paint_text(c, v, l, row, y);
    paint_link(c, v, l, row, y);
  }
  term_paint_cursor(c, v);
}
