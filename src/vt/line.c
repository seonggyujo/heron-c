// Making, filling and resizing lines of cells.
#include "vt/line.h"

#include <stdlib.h>
#include <string.h>

#include "base/mem.h"

Cell cell_blank(uint32_t bg) {
  Cell c = {0, COLOR_DEFAULT, bg, 0, 0};
  return c;
}

void line_init(Line *l, int len, Cell blank) {
  l->cells = xmalloc(sizeof(Cell) * (size_t)(len > 0 ? len : 1));
  l->len = len;
  l->wrapped = false;
  for (int i = 0; i < len; i++) l->cells[i] = blank;
}

void line_free(Line *l) {
  free(l->cells);
  l->cells = NULL;
  l->len = 0;
}

void line_fill(Line *l, int from, int to, Cell blank) {
  if (from < 0) from = 0;
  if (to > l->len) to = l->len;
  if (from >= to) return;
  if (from > 0 && (l->cells[from].attr & ATTR_WIDE_TAIL)) l->cells[from - 1] = blank;
  if (to < l->len && (l->cells[to].attr & ATTR_WIDE_TAIL)) l->cells[to] = blank;
  for (int i = from; i < to; i++) l->cells[i] = blank;
}

void line_fix_wide(Line *l, Cell blank) {
  for (int i = 0; i < l->len; i++) {
    uint16_t a = l->cells[i].attr;
    if ((a & ATTR_WIDE) && (i + 1 >= l->len || !(l->cells[i + 1].attr & ATTR_WIDE_TAIL))) {
      l->cells[i] = blank;
    } else if ((a & ATTR_WIDE_TAIL) && (i == 0 || !(l->cells[i - 1].attr & ATTR_WIDE))) {
      l->cells[i] = blank;
    }
  }
}

void line_resize(Line *l, int len, Cell blank) {
  if (len == l->len) return;
  l->cells = xrealloc(l->cells, sizeof(Cell) * (size_t)(len > 0 ? len : 1));
  for (int i = l->len; i < len; i++) l->cells[i] = blank;
  l->len = len;
  line_fix_wide(l, blank);
}

static bool cell_is_empty(Cell c) {
  return c.ch == 0 && c.bg == COLOR_DEFAULT &&
         !(c.attr & (ATTR_INVERSE | ATTR_UNDERLINE | ATTR_STRIKE));
}

void line_copy_trimmed(const Line *src, Line *dst) {
  int n = src->len;
  while (n > 0 && cell_is_empty(src->cells[n - 1])) n--;
  dst->cells = xmalloc(sizeof(Cell) * (size_t)(n > 0 ? n : 1));
  memcpy(dst->cells, src->cells, sizeof(Cell) * (size_t)n);
  dst->len = n;
  dst->wrapped = src->wrapped;
}
