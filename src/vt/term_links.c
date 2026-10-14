// Finding the link under a cell: an OSC 8 hyperlink first, then a plain URL
// in the logical line (rows joined by soft wraps, up to 16 of them).
#include <stdio.h>
#include <stdlib.h>

#include "base/mem.h"
#include "base/utf.h"
#include "vt/term_internal.h"
#include "vt/url_scan.h"

#define MAX_CHAIN 16

typedef struct {
  uint64_t row;
  int col;
} Spot;

static bool osc8_link(const Term *t, uint64_t row, int col, TermLink *out) {
  const Line *l = term_line_by_id(t, row);
  if (!l || col >= l->len || !l->cells[col].link) return false;
  uint16_t id = l->cells[col].link;
  const char *uri = term_link_uri(t, id);
  if (!uri) return false;
  int a = col, b = col;
  while (a > 0 && l->cells[a - 1].link == id) a--;
  while (b + 1 < l->len && l->cells[b + 1].link == id) b++;
  snprintf(out->url, sizeof out->url, "%s", uri);
  out->row0 = out->row1 = row;
  out->col0 = a;
  out->col1 = b;
  return true;
}

static void write_url(TermLink *out, const uint32_t *text, int start, int end) {
  size_t n = 0;
  for (int i = start; i < end; i++) {
    char u[4];
    int len = utf8_encode(text[i], u);
    if (n + (size_t)len >= sizeof out->url) break;
    for (int k = 0; k < len; k++) out->url[n++] = u[k];
  }
  out->url[n] = 0;
}

static bool plain_link(const Term *t, uint64_t row, int col, TermLink *out) {
  uint64_t first = row, last = row;
  const Line *l;
  while (row - first < MAX_CHAIN - 1 && first > 0 && (l = term_line_by_id(t, first - 1)) && l->wrapped) first--;
  while (last - first < MAX_CHAIN - 1 && (l = term_line_by_id(t, last)) && l->wrapped && term_line_by_id(t, last + 1)) {
    last++;
  }
  int cap = MAX_CHAIN * term_cols(t), n = 0, at = -1;
  uint32_t *text = xmalloc(sizeof(uint32_t) * (size_t)cap);
  Spot *spots = xmalloc(sizeof(Spot) * (size_t)cap);
  for (uint64_t r = first; r <= last; r++) {
    l = term_line_by_id(t, r);
    for (int c = 0; l && c < l->len && n < cap; c++) {
      bool tail = l->cells[c].attr & ATTR_WIDE_TAIL;
      if (r == row && c == col) at = tail ? n - 1 : n;
      if (tail) continue;
      text[n] = l->cells[c].ch ? l->cells[c].ch : ' ';
      spots[n++] = (Spot){r, c};
    }
  }
  int s, e;
  bool found = at >= 0 && url_find(text, n, at, &s, &e);
  if (found) {
    write_url(out, text, s, e);
    out->row0 = spots[s].row;
    out->col0 = spots[s].col;
    out->row1 = spots[e - 1].row;
    l = term_line_by_id(t, out->row1);
    out->col1 = spots[e - 1].col + (l && (l->cells[spots[e - 1].col].attr & ATTR_WIDE) ? 1 : 0);
  }
  free(text);
  free(spots);
  return found;
}

bool term_link_at(const Term *t, uint64_t row, int col, TermLink *out) {
  return osc8_link(t, row, col, out) || plain_link(t, row, col, out);
}
