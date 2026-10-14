// Cursor movement, scrolling and tab stops on the active screen.
#include <string.h>

#include "vt/line.h"
#include "vt/term_internal.h"

Cell term_blank(const Term *t) { return cell_blank(t->pen.bg); }

static void clear_line(Term *t, Line *l) {
  line_fill(l, 0, l->len, term_blank(t));
  l->wrapped = false;
}

// Moves rows [top, bottom] up by n and blanks the rows that open at the
// bottom. With to_history, rows leaving the top are kept in history.
void term_scroll_up(Term *t, int top, int bottom, int n, bool to_history) {
  int height = bottom - top + 1;
  if (n > height) n = height;
  for (int i = 0; i < n; i++) {
    Line gone = t->screen[top];
    if (to_history) {
      Line kept;
      line_copy_trimmed(&gone, &kept);
      scrollback_push(&t->history, kept);
      // Keep a scrolled-back view on the same lines.
      if (t->view_offset > 0 && t->view_offset < t->history.count) t->view_offset++;
    }
    memmove(&t->screen[top], &t->screen[top + 1], sizeof(Line) * (size_t)(height - 1));
    t->screen[bottom] = gone;
    clear_line(t, &t->screen[bottom]);
  }
}

void term_scroll_down(Term *t, int top, int bottom, int n) {
  int height = bottom - top + 1;
  if (n > height) n = height;
  for (int i = 0; i < n; i++) {
    Line gone = t->screen[bottom];
    memmove(&t->screen[top + 1], &t->screen[top], sizeof(Line) * (size_t)(height - 1));
    t->screen[top] = gone;
    clear_line(t, &t->screen[top]);
  }
}

// Line feed. Like xterm, lines scrolled out of a region that starts at the
// top of the main screen go to history.
void term_index(Term *t) {
  t->wrap_pending = false;
  if (t->y == t->bottom) {
    term_scroll_up(t, t->top, t->bottom, 1, !t->alt_active && t->top == 0);
  } else if (t->y < t->rows - 1) {
    t->y++;
  }
}

void term_reverse_index(Term *t) {
  t->wrap_pending = false;
  if (t->y == t->top) {
    term_scroll_down(t, t->top, t->bottom, 1);
  } else if (t->y > 0) {
    t->y--;
  }
}

static int clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void term_move_to(Term *t, int x, int y) {
  int top = t->origin ? t->top : 0;
  int bottom = t->origin ? t->bottom : t->rows - 1;
  t->x = clamp(x, 0, t->cols - 1);
  t->y = clamp(y + top, top, bottom);
  t->wrap_pending = false;
}

void term_tab(Term *t, int n) {
  for (; n > 0; n--) {
    int x = t->x + 1;
    while (x < t->cols - 1 && !t->tabs[x]) x++;
    t->x = clamp(x, 0, t->cols - 1);
  }
  for (; n < 0; n++) {
    int x = t->x - 1;
    while (x > 0 && !t->tabs[x]) x--;
    t->x = clamp(x, 0, t->cols - 1);
  }
  t->wrap_pending = false;
}

void term_reset_tabs(Term *t) {
  for (int x = 0; x < t->cols; x++) t->tabs[x] = x > 0 && x % 8 == 0;
}
