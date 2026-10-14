// Terminal tests: printing, wrapping, wide characters, scrolling, resizing.
#include <stdlib.h>

#include "check.h"
#include "term_helpers.h"
#include "vt/selection.h"

void test_vt_print_and_wrap(void) {
  Term *t = term_new(10, 3, 100);
  feed(t, "hello");
  CHECK_STR(row_text(t, 0), "hello");
  int x, y;
  term_cursor(t, &x, &y);
  CHECK(x == 5 && y == 0);
  feed(t, "\r\n0123456789AB");
  CHECK_STR(row_text(t, 1), "0123456789");
  CHECK_STR(row_text(t, 2), "AB");
  CHECK(term_view_line(t, 1)->wrapped);
  term_free(t);
}

void test_vt_wide_chars(void) {
  Term *t = term_new(5, 2, 0);
  feed(t, "\xea\xb0\x80\xeb\x82\x98");  // 가나
  const Line *l = term_view_line(t, 0);
  CHECK(l->cells[0].ch == 0xAC00 && (l->cells[0].attr & ATTR_WIDE));
  CHECK(l->cells[1].attr & ATTR_WIDE_TAIL);
  int x, y;
  term_cursor(t, &x, &y);
  CHECK(x == 4);
  feed(t, "\xeb\x8b\xa4");  // 다 does not fit in the last column: wraps
  CHECK_STR(row_text(t, 1), "\xeb\x8b\xa4");
  feed(t, "\x1b[1;2HZ");  // overwriting the tail of 가 blanks its head
  CHECK_STR(row_text(t, 0), " Z\xeb\x82\x98");
  term_free(t);
}

void test_vt_split_utf8(void) {
  Term *t = term_new(10, 1, 0);
  feed(t, "\xea\xb0");
  feed(t, "\x80!");
  CHECK_STR(row_text(t, 0), "\xea\xb0\x80!");
  term_free(t);
}

void test_vt_history(void) {
  Term *t = term_new(10, 3, 100);
  feed(t, "1\r\n2\r\n3\r\n4\r\n5");
  CHECK(term_history_size(t) == 2);
  CHECK_STR(row_text(t, 0), "3");
  term_scroll_view(t, 2);
  CHECK_STR(row_text(t, 0), "1");
  feed(t, "\r\n6");  // the view stays on the same lines while output arrives
  CHECK_STR(row_text(t, 0), "1");
  term_view_reset(t);
  CHECK_STR(row_text(t, 2), "6");
  term_free(t);
}

void test_vt_scroll_region(void) {
  Term *t = term_new(10, 4, 100);
  feed(t, "a\r\nb\r\nc\r\nd");
  feed(t, "\x1b[2;3r\x1b[3;1H\n");  // LF at the region bottom scrolls rows 2-3 only
  CHECK_STR(row_text(t, 0), "a");
  CHECK_STR(row_text(t, 1), "c");
  CHECK_STR(row_text(t, 2), "");
  CHECK_STR(row_text(t, 3), "d");
  CHECK(term_history_size(t) == 0);
  term_free(t);
}

void test_vt_alt_screen(void) {
  Term *t = term_new(10, 3, 100);
  feed(t, "main\x1b[?1049h");
  CHECK_STR(row_text(t, 0), "");
  feed(t, "alt");
  CHECK(term_modes(t).alt_screen);
  feed(t, "\x1b[?1049l");
  CHECK_STR(row_text(t, 0), "main");
  int x, y;
  term_cursor(t, &x, &y);
  CHECK(x == 4 && y == 0);
  term_free(t);
}

void test_vt_resize(void) {
  Term *t = term_new(10, 4, 100);
  feed(t, "1\r\n2\r\n3\r\n4");
  term_resize(t, 6, 2);  // cursor on the last row: top rows move to history
  CHECK(term_history_size(t) == 2);
  CHECK_STR(row_text(t, 0), "3");
  CHECK_STR(row_text(t, 1), "4");
  CHECK(term_cols(t) == 6);
  term_free(t);
}

void test_vt_selection(void) {
  Term *t = term_new(12, 3, 100);
  feed(t, "hello world \r\nsecond");
  Selection s;
  selection_begin(&s, (TextPos){term_view_row_id(t, 0), 6});
  selection_extend(&s, (TextPos){term_view_row_id(t, 1), 2});
  char *text = selection_text(&s, t);
  CHECK_STR(text, "world\r\nsec");
  free(text);
  selection_word(&s, t, (TextPos){term_view_row_id(t, 0), 1});
  text = selection_text(&s, t);
  CHECK_STR(text, "hello");
  free(text);
  term_free(t);
}
