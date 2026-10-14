// Helpers for terminal tests.
#include "term_helpers.h"

#include <string.h>

#include "base/utf.h"

void feed(Term *t, const char *s) { term_feed(t, (const uint8_t *)s, strlen(s)); }

const char *row_text(const Term *t, int y) {
  static char text[1024];
  size_t n = 0, keep = 0;
  const Line *l = term_view_line(t, y);
  for (int x = 0; x < l->len && n + 4 < sizeof text; x++) {
    const Cell *c = &l->cells[x];
    if (c->attr & ATTR_WIDE_TAIL) continue;
    char u[4];
    int len = utf8_encode(c->ch ? c->ch : ' ', u);
    memcpy(text + n, u, (size_t)len);
    n += (size_t)len;
    if (c->ch) keep = n;
  }
  text[keep] = 0;
  return text;
}

const char *take_replies(Term *t) {
  static char text[256];
  Buf *r = term_replies(t);
  size_t n = r->len < sizeof text - 1 ? r->len : sizeof text - 1;
  memcpy(text, r->data, n);
  text[n] = 0;
  buf_clear(r);
  return text;
}
