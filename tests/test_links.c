// Tests for URL scanning and the link under a cell.

#include "check.h"
#include "term_helpers.h"
#include "vt/url_scan.h"

static int to_cps(const char *ascii, uint32_t *out) {
  int n = 0;
  for (const char *p = ascii; *p; p++) out[n++] = (uint8_t)*p;
  return n;
}

void test_url_scan(void) {
  uint32_t t[128];
  int s, e, n = to_cps("see https://example.com/a?b=1. done", t);
  CHECK(url_find(t, n, 10, &s, &e) && s == 4 && e == 29);  // the final period is not part of it
  CHECK(!url_find(t, n, 1, &s, &e));
  n = to_cps("(http://x.io/p) http://", t);
  CHECK(url_find(t, n, 5, &s, &e) && s == 1 && e == 14);  // stops before ')'
  CHECK(!url_find(t, n, 20, &s, &e));                    // nothing after "//"
  n = to_cps("HTTPS://UP.CASE", t);
  CHECK(url_find(t, n, 0, &s, &e) && e == n);
}

void test_term_links(void) {
  Term *t = term_new(20, 4, 10);
  feed(t, "go https://a.io/xxxxxxxxxxxxxx end");  // wraps onto the next row
  TermLink l;
  CHECK(term_link_at(t, term_view_row_id(t, 1), 3, &l));
  CHECK_STR(l.url, "https://a.io/xxxxxxxxxxxxxx");
  CHECK(l.row0 == term_view_row_id(t, 0) && l.col0 == 3 && l.row1 == term_view_row_id(t, 1) && l.col1 == 9);
  CHECK(!term_link_at(t, term_view_row_id(t, 1), 12, &l));
  feed(t, "\r\n\x1b]8;;https://osc.example/\x07link\x1b]8;;\x07 after");
  CHECK(term_link_at(t, term_view_row_id(t, 2), 1, &l));
  CHECK_STR(l.url, "https://osc.example/");
  CHECK(l.col0 == 0 && l.col1 == 3);
  CHECK(!term_link_at(t, term_view_row_id(t, 2), 6, &l));
  term_free(t);
}
