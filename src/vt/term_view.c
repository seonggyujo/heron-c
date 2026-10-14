// Looking back through history, and stable row ids for selections.
#include "vt/term_internal.h"

int term_history_size(const Term *t) { return t->alt_active ? 0 : t->history.count; }

int term_view_offset(const Term *t) { return t->view_offset; }

void term_scroll_view(Term *t, int lines) {
  int offset = t->view_offset + lines;
  int max = term_history_size(t);
  t->view_offset = offset < 0 ? 0 : offset > max ? max : offset;
}

void term_view_reset(Term *t) { t->view_offset = 0; }

const Line *term_view_line(const Term *t, int y) {
  int i = y - t->view_offset;  // row on the live screen; negative is history
  if (i >= 0) return &t->screen[i];
  return scrollback_line(&t->history, t->history.count + i);
}

uint64_t term_view_row_id(const Term *t, int y) {
  return t->history.pushed + (uint64_t)(int64_t)(y - t->view_offset);
}

const Line *term_line_by_id(const Term *t, uint64_t id) {
  uint64_t first = t->history.pushed - (uint64_t)t->history.count;
  if (id < first) return NULL;
  if (id < t->history.pushed) return scrollback_line(&t->history, (int)(id - first));
  uint64_t y = id - t->history.pushed;
  return y < (uint64_t)t->rows ? &t->screen[y] : NULL;
}
