// Finding runs of blank cells in what a terminal shows.
#include "buddy/ground.h"

#include <stdlib.h>

// Attributes that draw something even on a space.
#define INK_ATTRS (ATTR_INVERSE | ATTR_UNDERLINE | ATTR_STRIKE | ATTR_WIDE | ATTR_WIDE_TAIL)
// Cells after the cursor kept clear for what is being typed.
#define CURSOR_ROOM 2

static bool near_cursor(const Term *t, int col, int row) {
  if (term_view_offset(t) != 0 || !term_modes(t).cursor_visible) return false;
  int x, y;
  term_cursor(t, &x, &y);
  return row == y && col >= x && col <= x + CURSOR_ROOM;
}

bool ground_free(const Term *t, int col, int row) {
  if (col < 0 || row < 0 || col >= term_cols(t) || row >= term_rows(t)) return false;
  const Line *l = term_view_line(t, row);
  if (l && col < l->len) {
    Cell c = l->cells[col];
    if ((c.ch && c.ch != ' ') || c.bg != COLOR_DEFAULT || (c.attr & INK_ATTRS)) return false;
  }
  return !near_cursor(t, col, row);
}

// Column col is free in the h rows ending at row.
static bool column_free(const Term *t, int col, int row, int h) {
  for (int y = row - h + 1; y <= row; y++) {
    if (!ground_free(t, col, y)) return false;
  }
  return true;
}

bool ground_run(const Term *t, int row, int h, int col0, int col1, Spot *out) {
  int cols = term_cols(t);
  if (row - h + 1 < 0 || row >= term_rows(t) || col0 < 0 || col1 >= cols) return false;
  for (int c = col0; c <= col1; c++) {
    if (!column_free(t, c, row, h)) return false;
  }
  int c0 = col0, c1 = col1 + 1;
  while (c0 > 0 && column_free(t, c0 - 1, row, h)) c0--;
  while (c1 < cols && column_free(t, c1, row, h)) c1++;
  *out = (Spot){row, c0, c1};
  return true;
}

typedef struct {
  FindMode mode;
  int w, col, row;
  uint32_t pick;
  uint32_t seen;
  bool found;
  Spot best;
  int best_score;
} Finder;

// How far a heron at (col, row) goes to stand in s; a row is about two
// columns high.
static int distance(const Finder *f, Spot s) {
  int last = s.c1 - f->w;
  int dc = f->col < s.c0 ? s.c0 - f->col : f->col > last ? f->col - last : 0;
  return abs(s.row - f->row) * 2 + dc;
}

// Gives every run to f; ties go to the lower, then the righter run.
static void take(Finder *f, Spot s) {
  int score;
  if (f->mode == FIND_ANY) {
    score = f->seen++ == f->pick ? 1 : 0;
    if (!score) return;
  } else {
    score = f->mode == FIND_HOME ? s.row : -distance(f, s);
  }
  if (!f->found || score >= f->best_score) {
    f->found = true;
    f->best = s;
    f->best_score = score;
  }
}

// Every run at least f->w wide and h tall, top to bottom, left to right.
static void each_run(const Term *t, int h, Finder *f, bool count_only) {
  int cols = term_cols(t);
  for (int row = h - 1; row < term_rows(t); row++) {
    int c = 0;
    while (c < cols) {
      while (c < cols && !column_free(t, c, row, h)) c++;
      int start = c;
      while (c < cols && column_free(t, c, row, h)) c++;
      if (c - start < f->w) continue;
      if (count_only) {
        f->seen++;
      } else {
        take(f, (Spot){row, start, c});
      }
    }
  }
}

bool ground_find(const Term *t, int w, int h, FindMode mode, int col, int row, uint32_t pick, Spot *out) {
  Finder f = {mode, w, col, row, 0, 0, false, {0, 0, 0}, 0};
  if (mode == FIND_ANY) {
    each_run(t, h, &f, true);
    if (!f.seen) return false;
    f.pick = pick % f.seen;
    f.seen = 0;
  }
  each_run(t, h, &f, false);
  if (f.found) *out = f.best;
  return f.found;
}
