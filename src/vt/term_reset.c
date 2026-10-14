// Soft reset (DECSTR, CSI ! p) and full reset (RIS, ESC c).
#include "vt/line.h"
#include "vt/term_internal.h"

static void reset_saved(SavedCursor *s) {
  s->x = s->y = 0;
  s->pen = cell_blank(COLOR_DEFAULT);
  s->origin = s->g0_graphics = false;
}

void term_soft_reset(Term *t) {
  t->modes.cursor_visible = true;
  t->modes.app_cursor = false;
  t->modes.app_keypad = false;
  t->insert = false;
  t->origin = false;
  t->autowrap = true;
  t->g0_graphics = false;
  t->top = 0;
  t->bottom = t->rows - 1;
  t->pen = cell_blank(COLOR_DEFAULT);
  t->wrap_pending = false;
  reset_saved(&t->saved_main);
  reset_saved(&t->saved_alt);
}

void term_full_reset(Term *t) {
  t->alt_active = false;
  t->screen = t->main_lines;
  t->modes = (TermModes){0};
  term_soft_reset(t);
  t->newline_mode = false;
  for (int y = 0; y < t->rows; y++) {
    line_fill(&t->main_lines[y], 0, t->cols, cell_blank(COLOR_DEFAULT));
    line_fill(&t->alt_lines[y], 0, t->cols, cell_blank(COLOR_DEFAULT));
    t->main_lines[y].wrapped = t->alt_lines[y].wrapped = false;
  }
  t->x = t->y = 0;
  t->view_offset = 0;
  t->last_char = 0;
  term_reset_tabs(t);
}
