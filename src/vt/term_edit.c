// Erasing, inserting and deleting characters and lines at the cursor.
#include <string.h>

#include "vt/line.h"
#include "vt/term_internal.h"

void term_erase_in_line(Term *t, int mode) {
  Line *l = &t->screen[t->y];
  switch (mode) {
    case 0:
      line_fill(l, t->x, t->cols, term_blank(t));
      l->wrapped = false;
      break;
    case 1:
      line_fill(l, 0, t->x + 1, term_blank(t));
      break;
    case 2:
      line_fill(l, 0, t->cols, term_blank(t));
      l->wrapped = false;
      break;
  }
}

static void erase_rows(Term *t, int from, int to) {
  for (int y = from; y < to; y++) {
    line_fill(&t->screen[y], 0, t->cols, term_blank(t));
    t->screen[y].wrapped = false;
  }
}

void term_erase_in_display(Term *t, int mode) {
  switch (mode) {
    case 0:
      term_erase_in_line(t, 0);
      erase_rows(t, t->y + 1, t->rows);
      break;
    case 1:
      erase_rows(t, 0, t->y);
      term_erase_in_line(t, 1);
      break;
    case 2:
      erase_rows(t, 0, t->rows);
      break;
    case 3:
      scrollback_clear(&t->history);
      t->view_offset = 0;
      break;
  }
}

void term_erase_chars(Term *t, int n) {
  line_fill(&t->screen[t->y], t->x, t->x + n, term_blank(t));
  t->wrap_pending = false;
}

void term_insert_chars(Term *t, int n) {
  Line *l = &t->screen[t->y];
  int x = t->x;
  if (n > t->cols - x) n = t->cols - x;
  memmove(&l->cells[x + n], &l->cells[x], sizeof(Cell) * (size_t)(t->cols - x - n));
  for (int i = x; i < x + n; i++) l->cells[i] = term_blank(t);
  line_fix_wide(l, term_blank(t));
  t->wrap_pending = false;
}

void term_delete_chars(Term *t, int n) {
  Line *l = &t->screen[t->y];
  int x = t->x;
  if (n > t->cols - x) n = t->cols - x;
  memmove(&l->cells[x], &l->cells[x + n], sizeof(Cell) * (size_t)(t->cols - x - n));
  for (int i = t->cols - n; i < t->cols; i++) l->cells[i] = term_blank(t);
  line_fix_wide(l, term_blank(t));
  t->wrap_pending = false;
}

void term_insert_lines(Term *t, int n) {
  if (t->y < t->top || t->y > t->bottom) return;
  term_scroll_down(t, t->y, t->bottom, n);
  t->x = 0;
  t->wrap_pending = false;
}

void term_delete_lines(Term *t, int n) {
  if (t->y < t->top || t->y > t->bottom) return;
  term_scroll_up(t, t->y, t->bottom, n, false);
  t->x = 0;
  t->wrap_pending = false;
}
