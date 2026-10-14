// Changing the screen size. Lines are cut or padded, not reflowed: ConPTY
// repaints the whole screen after a resize anyway.
#include <stdlib.h>

#include "base/mem.h"
#include "vt/line.h"
#include "vt/term_internal.h"

static int clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

// When rows shrink, empty room below the cursor goes first, then lines from
// the top (into history on the main screen) so the cursor stays on screen.
static void resize_lines(Term *t, Line **lines, int cols, int rows, int *cursor_y, bool to_history) {
  Line *old = *lines;
  int old_rows = t->rows;
  int shift = 0;
  if (rows < old_rows) {
    int extra = old_rows - rows;
    int below = old_rows - 1 - *cursor_y;
    shift = extra - (below < extra ? below : extra);
  }
  for (int y = 0; y < shift; y++) {
    if (to_history) {
      Line kept;
      line_copy_trimmed(&old[y], &kept);
      scrollback_push(&t->history, kept);
    }
    line_free(&old[y]);
  }
  Line *fresh = xcalloc((size_t)rows, sizeof(Line));
  Cell blank = cell_blank(COLOR_DEFAULT);
  for (int y = 0; y < rows; y++) {
    int src = y + shift;
    if (src < old_rows) {
      fresh[y] = old[src];
      line_resize(&fresh[y], cols, blank);
    } else {
      line_init(&fresh[y], cols, blank);
    }
  }
  for (int y = shift + rows; y < old_rows; y++) line_free(&old[y]);
  free(old);
  *lines = fresh;
  *cursor_y = clamp(*cursor_y - shift, 0, rows - 1);
}

void term_resize(Term *t, int cols, int rows) {
  cols = cols > 2 ? cols : 2;
  rows = rows > 1 ? rows : 1;
  if (cols == t->cols && rows == t->rows) return;
  int main_y = t->alt_active ? t->saved_main.y : t->y;
  int alt_y = t->alt_active ? t->y : t->saved_alt.y;
  resize_lines(t, &t->main_lines, cols, rows, &main_y, true);
  resize_lines(t, &t->alt_lines, cols, rows, &alt_y, false);
  t->screen = t->alt_active ? t->alt_lines : t->main_lines;
  if (t->alt_active) {
    t->y = alt_y;
    t->saved_main.y = main_y;
  } else {
    t->y = main_y;
    t->saved_alt.y = alt_y;
  }
  t->tabs = xrealloc(t->tabs, sizeof(bool) * (size_t)cols);
  t->cols = cols;
  t->rows = rows;
  term_reset_tabs(t);
  t->x = clamp(t->x, 0, cols - 1);
  t->saved_main.x = clamp(t->saved_main.x, 0, cols - 1);
  t->saved_alt.x = clamp(t->saved_alt.x, 0, cols - 1);
  t->top = 0;
  t->bottom = rows - 1;
  t->wrap_pending = false;
  t->view_offset = clamp(t->view_offset, 0, term_history_size(t));
}
