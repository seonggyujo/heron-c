// Selection, history scrolling and mouse reports for a pane.
#include "pane/pane_mouse.h"

#include <stdlib.h>

#define WHEEL_LINES 3

static int clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

static void cell_at(const Pane *p, int x, int y, int *col, int *row) {
  int dx = x - p->body.x, dy = y - p->body.y;
  *col = clamp(dx < 0 ? -1 : dx / p->cell_w, 0, term_cols(p->term) - 1);
  *row = clamp(dy < 0 ? -1 : dy / p->cell_h, 0, term_rows(p->term) - 1);
}

bool pane_tracks_mouse(const Pane *p, KeyMods mods) {
  return !mods.shift && pane_running(p) && term_modes(p->term).mouse != 0;
}

static void report(Pane *p, MouseButton b, MouseAction action, int x, int y, KeyMods mods) {
  int col, row;
  cell_at(p, x, y, &col, &row);
  if (action == MOUSE_MOTION && col == p->last_col && row == p->last_row) return;
  p->last_col = col;
  p->last_row = row;
  char out[32];
  int n = mouse_report(b, action, col, row, mods, term_modes(p->term).mouse_sgr, out);
  if (n > 0) pty_write(p->pty, out, (size_t)n);
}

void pane_mouse_down(Pane *p, MouseButton b, int x, int y, KeyMods mods, bool double_click) {
  if (pane_tracks_mouse(p, mods)) {
    p->buttons_down++;
    report(p, b, MOUSE_PRESS, x, y, mods);
    return;
  }
  if (b != MOUSE_LEFT) return;
  int col, row;
  cell_at(p, x, y, &col, &row);
  TextPos at = {term_view_row_id(p->term, row), col};
  if (double_click) {
    selection_word(&p->selection, p->term, at);
    p->selecting = false;
  } else {
    selection_begin(&p->selection, at);
    p->selecting = true;
  }
}

void pane_mouse_move(Pane *p, int x, int y, KeyMods mods) {
  if (p->selecting) {
    int col, row;
    cell_at(p, x, y, &col, &row);
    selection_extend(&p->selection, (TextPos){term_view_row_id(p->term, row), col});
    return;
  }
  if (!pane_tracks_mouse(p, mods)) return;
  if (!mouse_wanted(term_modes(p->term).mouse, MOUSE_MOTION, p->buttons_down > 0)) return;
  report(p, p->buttons_down > 0 ? MOUSE_LEFT : MOUSE_NO_BUTTON, MOUSE_MOTION, x, y, mods);
}

void pane_mouse_up(Pane *p, MouseButton b, int x, int y, KeyMods mods) {
  if (p->selecting) {
    p->selecting = false;
    return;
  }
  if (p->buttons_down > 0) p->buttons_down--;
  if (pane_tracks_mouse(p, mods)) report(p, b, MOUSE_RELEASE, x, y, mods);
}

void pane_wheel(Pane *p, int notches, int x, int y, KeyMods mods) {
  if (notches == 0) return;
  if (pane_tracks_mouse(p, mods)) {
    for (int i = 0; i < abs(notches); i++) {
      report(p, notches > 0 ? MOUSE_WHEEL_UP : MOUSE_WHEEL_DOWN, MOUSE_PRESS, x, y, mods);
    }
    return;
  }
  TermModes m = term_modes(p->term);
  if (m.alt_screen && pane_running(p)) {
    // Full-screen programs without mouse tracking get arrow keys, as in
    // xterm.js, so the wheel still scrolls a pager.
    const char *up = m.app_cursor ? "\x1bOA" : "\x1b[A", *down = m.app_cursor ? "\x1bOB" : "\x1b[B";
    for (int i = 0; i < abs(notches) * WHEEL_LINES; i++) pty_write(p->pty, notches > 0 ? up : down, 3);
    return;
  }
  term_scroll_view(p->term, notches * WHEEL_LINES);
}
