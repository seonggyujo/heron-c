// Making and freeing a Term, feeding it bytes, and the read-only accessors.
#include <stdlib.h>

#include "base/mem.h"
#include "vt/line.h"
#include "vt/term_internal.h"

static Line *new_screen(int cols, int rows) {
  Line *lines = xcalloc((size_t)rows, sizeof(Line));
  for (int y = 0; y < rows; y++) line_init(&lines[y], cols, cell_blank(COLOR_DEFAULT));
  return lines;
}

static void free_screen(Line *lines, int rows) {
  for (int y = 0; y < rows; y++) line_free(&lines[y]);
  free(lines);
}

Term *term_new(int cols, int rows, int history_lines) {
  Term *t = xcalloc(1, sizeof *t);
  t->cols = cols > 2 ? cols : 2;
  t->rows = rows > 1 ? rows : 1;
  t->main_lines = new_screen(t->cols, t->rows);
  t->alt_lines = new_screen(t->cols, t->rows);
  t->screen = t->main_lines;
  t->tabs = xcalloc((size_t)t->cols, sizeof(bool));
  scrollback_init(&t->history, history_lines);
  term_full_reset(t);
  VtHandler h = {t, term_print, term_execute, term_esc, term_csi, term_osc};
  vt_parser_init(&t->parser, h);
  return t;
}

void term_free(Term *t) {
  if (!t) return;
  free_screen(t->main_lines, t->rows);
  free_screen(t->alt_lines, t->rows);
  scrollback_free(&t->history);
  buf_free(&t->replies);
  term_free_links(t);
  free(t->tabs);
  free(t);
}

void term_feed(Term *t, const uint8_t *data, size_t len) { vt_parser_feed(&t->parser, data, len); }

int term_cols(const Term *t) { return t->cols; }
int term_rows(const Term *t) { return t->rows; }
TermModes term_modes(const Term *t) { return t->modes; }

void term_cursor(const Term *t, int *x, int *y) {
  *x = t->x;
  *y = t->y;
}

const char *term_title(const Term *t) { return t->title; }
const char *term_cwd(const Term *t) { return t->cwd; }

unsigned term_take_events(Term *t) {
  unsigned e = t->events;
  t->events = 0;
  return e;
}

Buf *term_replies(Term *t) { return &t->replies; }
