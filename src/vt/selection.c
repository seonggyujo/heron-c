// Mouse selection: which cells are selected and the text they hold.
#include "vt/selection.h"

#include <string.h>

#include "base/buf.h"
#include "base/utf.h"

static bool before(TextPos a, TextPos b) { return a.row < b.row || (a.row == b.row && a.col < b.col); }

static void ordered(const Selection *s, TextPos *start, TextPos *end) {
  bool flip = before(s->head, s->anchor);
  *start = flip ? s->head : s->anchor;
  *end = flip ? s->anchor : s->head;
}

void selection_begin(Selection *s, TextPos at) {
  s->active = false;
  s->anchor = s->head = at;
}

void selection_extend(Selection *s, TextPos to) {
  s->head = to;
  if (to.row != s->anchor.row || to.col != s->anchor.col) s->active = true;
}

void selection_clear(Selection *s) { s->active = false; }

static bool is_word_char(uint32_t ch) {
  return ch > ' ' && !strchr("()[]{}<>'\"`,;|", (int)(ch < 128 ? ch : 'a'));
}

void selection_word(Selection *s, const Term *t, TextPos at) {
  const Line *l = term_line_by_id(t, at.row);
  if (!l || at.col >= l->len || !is_word_char(l->cells[at.col].ch)) return;
  int a = at.col, b = at.col;
  while (a > 0 && (is_word_char(l->cells[a - 1].ch) || (l->cells[a - 1].attr & ATTR_WIDE_TAIL))) a--;
  while (b + 1 < l->len && (is_word_char(l->cells[b + 1].ch) || (l->cells[b + 1].attr & ATTR_WIDE_TAIL))) b++;
  s->anchor = (TextPos){at.row, a};
  s->head = (TextPos){at.row, b};
  s->active = true;
}

bool selection_contains(const Selection *s, uint64_t row, int col) {
  if (!s->active) return false;
  TextPos start, end;
  ordered(s, &start, &end);
  TextPos p = {row, col};
  return !before(p, start) && !before(end, p);
}

static void trim_spaces(Buf *b, size_t floor) {
  while (b->len > floor && b->data[b->len - 1] == ' ') b->len--;
}

char *selection_text(const Selection *s, const Term *t) {
  if (!s->active) return NULL;
  TextPos start, end;
  ordered(s, &start, &end);
  Buf out = {0};
  for (uint64_t row = start.row; row <= end.row; row++) {
    const Line *l = term_line_by_id(t, row);
    if (!l) continue;
    int first = row == start.row ? start.col : 0;
    int last = row == end.row ? end.col : term_cols(t) - 1;
    size_t line_start = out.len;
    for (int c = first; c <= last && c < l->len; c++) {
      if (l->cells[c].attr & ATTR_WIDE_TAIL) continue;
      char u[4];
      uint32_t ch = l->cells[c].ch;
      buf_append(&out, u, (size_t)utf8_encode(ch ? ch : ' ', u));
    }
    if (row == end.row) {
      trim_spaces(&out, line_start);
    } else if (!l->wrapped) {
      trim_spaces(&out, line_start);
      buf_append_str(&out, "\r\n");
    }
  }
  buf_append(&out, "", 1);
  return out.data;
}
