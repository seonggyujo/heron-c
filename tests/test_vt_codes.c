// Terminal tests: escape sequences for cursor, erase, colors, modes, replies.
#include "check.h"
#include "term_helpers.h"

void test_vt_cursor_and_erase(void) {
  Term *t = term_new(10, 4, 0);
  feed(t, "\x1b[3;5HX");
  CHECK_STR(row_text(t, 2), "    X");
  feed(t, "\x1b[1;1Habcdef\x1b[3G\x1b[K");
  CHECK_STR(row_text(t, 0), "ab");
  feed(t, "\x1b[2;1Habcdef\x1b[2;2H\x1b[2P");  // delete 2 chars
  CHECK_STR(row_text(t, 1), "adef");
  feed(t, "\x1b[2;2H\x1b[2@");  // insert 2 blanks
  CHECK_STR(row_text(t, 1), "a  def");
  feed(t, "\x1b[2J");
  CHECK_STR(row_text(t, 2), "");
  term_free(t);
}

void test_vt_colors(void) {
  Term *t = term_new(10, 1, 0);
  feed(t, "\x1b[1;31mA\x1b[38;2;10;20;30mB\x1b[38:2::1:2:3mC\x1b[0;48;5;200mD\x1b[mE");
  const Line *l = term_view_line(t, 0);
  CHECK(l->cells[0].fg == COLOR_INDEX(1) && (l->cells[0].attr & ATTR_BOLD));
  CHECK(l->cells[1].fg == COLOR_RGB(10, 20, 30));
  CHECK(l->cells[2].fg == COLOR_RGB(1, 2, 3));
  CHECK(l->cells[3].bg == COLOR_INDEX(200) && l->cells[3].fg == COLOR_DEFAULT);
  CHECK(l->cells[3].attr == 0);
  CHECK(l->cells[4].bg == COLOR_DEFAULT);
  term_free(t);
}

void test_vt_replies(void) {
  Term *t = term_new(10, 4, 0);
  feed(t, "\x1b[2;3H\x1b[6n");
  CHECK_STR(take_replies(t), "\x1b[2;3R");
  feed(t, "\x1b[c");
  CHECK_STR(take_replies(t), "\x1b[?1;2c");
  feed(t, "\x1b[?2004h\x1b[?2004$p\x1b[?9999$p");
  CHECK_STR(take_replies(t), "\x1b[?2004;1$y\x1b[?9999;0$y");
  feed(t, "\x1b]11;?\x1b\\");
  CHECK_STR(take_replies(t), "\x1b]11;rgb:0000/0000/0000\x1b\\");
  term_free(t);
}

void test_vt_modes(void) {
  Term *t = term_new(10, 2, 0);
  feed(t, "\x1b[?1h\x1b[?25l\x1b[?1002h\x1b[?1006h\x1b[5 q");
  TermModes m = term_modes(t);
  CHECK(m.app_cursor && !m.cursor_visible && m.mouse == 1002 && m.mouse_sgr);
  CHECK(m.cursor_style == 5);
  feed(t, "\x1b[?1002l\x1b[!p");
  m = term_modes(t);
  CHECK(m.mouse == 0 && m.cursor_visible && !m.app_cursor);
  term_free(t);
}

void test_vt_osc(void) {
  Term *t = term_new(10, 2, 0);
  feed(t, "\x1b]0;\xec\xa0\x9c\xeb\xaa\xa9\x07");  // 제목
  CHECK_STR(term_title(t), "\xec\xa0\x9c\xeb\xaa\xa9");
  CHECK(term_take_events(t) == TERM_EVENT_TITLE);
  feed(t, "\x1b]9;9;C:\\Users\\home\x1b\\");
  CHECK_STR(term_cwd(t), "C:\\Users\\home");
  feed(t, "\x1b]9;9;/c/Users/home\x07");
  CHECK_STR(term_cwd(t), "C:\\Users\\home");
  CHECK(term_take_events(t) == (TERM_EVENT_CWD | TERM_EVENT_PROMPT));  // the second report changed nothing
  feed(t, "\x1b]9;9;C:\\Users\\home\x07");
  CHECK(term_take_events(t) == TERM_EVENT_PROMPT);  // each report is a prompt
  feed(t, "\x1b]7;file://pc/D:/my%20dir\x07");
  CHECK_STR(term_cwd(t), "D:\\my dir");
  term_free(t);
}

void test_vt_charset_and_tabs(void) {
  Term *t = term_new(20, 1, 0);
  feed(t, "\x1b(0qx\x1b(Bq\tT");
  CHECK_STR(row_text(t, 0), "\xe2\x94\x80\xe2\x94\x82q     T");
  term_free(t);
}
