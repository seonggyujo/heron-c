// CSI sequences that move the cursor, edit the screen or set the scroll
// region. Sequences that answer back are in term_report.c.
#include "vt/term_internal.h"

static int min_int(int a, int b) { return a < b ? a : b; }
static int max_int(int a, int b) { return a > b ? a : b; }

// Up and down stop at the scroll region edge when the cursor is inside it.
static void cursor_up(Term *t, int n) {
  int limit = t->y >= t->top ? t->top : 0;
  t->y = max_int(limit, t->y - n);
  t->wrap_pending = false;
}

static void cursor_down(Term *t, int n) {
  int limit = t->y <= t->bottom ? t->bottom : t->rows - 1;
  t->y = min_int(limit, t->y + n);
  t->wrap_pending = false;
}

static void set_column(Term *t, int x) {
  t->x = max_int(0, min_int(t->cols - 1, x));
  t->wrap_pending = false;
}

static void set_scroll_region(Term *t, const VtParams *p) {
  int top = vt_count(p, 0) - 1;
  int bottom = vt_param(p, 1, 0);
  bottom = (bottom <= 0 || bottom > t->rows) ? t->rows - 1 : bottom - 1;
  if (top >= bottom) return;
  t->top = top;
  t->bottom = bottom;
  term_move_to(t, 0, 0);
}

static void clear_tabs(Term *t, int mode) {
  if (mode == 0) t->tabs[t->x] = false;
  if (mode == 3) {
    for (int x = 0; x < t->cols; x++) t->tabs[x] = false;
  }
}

static void repeat_last(Term *t, int n) {
  if (!t->last_char) return;
  for (int i = 0; i < n && i < 65535; i++) term_print(t, t->last_char);
}

void term_csi(void *user, char prefix, const VtParams *p, const char *inter, uint8_t final) {
  Term *t = user;
  if (inter[0] == ' ' && final == 'q' && !prefix) {
    t->modes.cursor_style = vt_param(p, 0, 0);
    return;
  }
  if (inter[0] == '!' && final == 'p') {
    term_soft_reset(t);
    return;
  }
  if (inter[0] == '$' && final == 'p') {
    term_report(t, prefix, p, 'p');
    return;
  }
  if (inter[0]) return;
  if (prefix == '?' && (final == 'h' || final == 'l')) {
    term_set_modes(t, '?', p, final == 'h');
    return;
  }
  if (prefix) {
    term_report(t, prefix, p, final);
    return;
  }
  int n = vt_count(p, 0);
  switch (final) {
    case '@': term_insert_chars(t, n); break;
    case 'A': cursor_up(t, n); break;
    case 'B':
    case 'e': cursor_down(t, n); break;
    case 'C':
    case 'a': set_column(t, t->x + n); break;
    case 'D': set_column(t, t->x - n); break;
    case 'E': cursor_down(t, n); t->x = 0; break;
    case 'F': cursor_up(t, n); t->x = 0; break;
    case 'G':
    case '`': set_column(t, n - 1); break;
    case 'H':
    case 'f': term_move_to(t, vt_count(p, 1) - 1, n - 1); break;
    case 'I': term_tab(t, n); break;
    case 'J': term_erase_in_display(t, vt_param(p, 0, 0)); break;
    case 'K': term_erase_in_line(t, vt_param(p, 0, 0)); break;
    case 'L': term_insert_lines(t, n); break;
    case 'M': term_delete_lines(t, n); break;
    case 'P': term_delete_chars(t, n); break;
    case 'S': term_scroll_up(t, t->top, t->bottom, n, false); break;
    case 'T':
      if (p->n <= 1) term_scroll_down(t, t->top, t->bottom, n);
      break;
    case 'X': term_erase_chars(t, n); break;
    case 'Z': term_tab(t, -n); break;
    case 'b': repeat_last(t, n); break;
    case 'd': term_move_to(t, t->x, n - 1); break;
    case 'g': clear_tabs(t, vt_param(p, 0, 0)); break;
    case 'h':
    case 'l': term_set_modes(t, 0, p, final == 'h'); break;
    case 'm': term_sgr(t, p); break;
    case 'r': set_scroll_region(t, p); break;
    case 's':
      if (p->n == 0) term_save_cursor(t);
      break;
    case 'u': term_restore_cursor(t); break;
    case 'c':
    case 'n':
    case 't': term_report(t, 0, p, final); break;
  }
}
