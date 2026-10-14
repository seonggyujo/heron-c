// Parser callbacks for printable characters, C0 controls and ESC sequences.
#include "vt/charset.h"
#include "vt/line.h"
#include "vt/term_internal.h"
#include "vt/wcwidth.h"

// Wraps to the start of the next line, marking the line as soft-wrapped.
static void wrap(Term *t) {
  t->screen[t->y].wrapped = true;
  t->x = 0;
  term_index(t);
}

void term_print(void *user, uint32_t cp) {
  Term *t = user;
  if (t->g0_graphics) cp = charset_dec_graphics(cp);
  int w = vt_wcwidth(cp);
  if (w == 0) return;  // combining marks are not stored
  if (t->wrap_pending) {
    t->wrap_pending = false;
    if (t->autowrap) wrap(t);
  }
  if (w == 2 && t->x == t->cols - 1) {
    if (!t->autowrap) return;
    line_fill(&t->screen[t->y], t->x, t->cols, term_blank(t));
    wrap(t);
  }
  if (t->insert) term_insert_chars(t, w);

  Line *l = &t->screen[t->y];
  int x = t->x;
  // Writing over half of a wide character blanks its other half.
  line_fill(l, x, x + w, term_blank(t));
  Cell c = t->pen;
  c.ch = cp;
  c.attr = (uint16_t)((c.attr & ~(ATTR_WIDE | ATTR_WIDE_TAIL)) | (w == 2 ? ATTR_WIDE : 0));
  l->cells[x] = c;
  if (w == 2) {
    Cell tail = c;
    tail.ch = 0;
    tail.attr = (uint16_t)((c.attr & ~ATTR_WIDE) | ATTR_WIDE_TAIL);
    l->cells[x + 1] = tail;
  }
  t->last_char = cp;
  if (x + w >= t->cols) {
    t->x = t->cols - 1;
    t->wrap_pending = t->autowrap;
  } else {
    t->x = x + w;
  }
}

void term_execute(void *user, uint8_t control) {
  Term *t = user;
  switch (control) {
    case 0x07:  // BEL
      t->events |= TERM_EVENT_BELL;
      break;
    case 0x08:  // BS: a pending wrap is cancelled in place
      if (t->wrap_pending) {
        t->wrap_pending = false;
      } else if (t->x > 0) {
        t->x--;
      }
      break;
    case 0x09:  // HT
      term_tab(t, 1);
      break;
    case 0x0A:  // LF, VT, FF
    case 0x0B:
    case 0x0C:
      term_index(t);
      if (t->newline_mode) t->x = 0;
      break;
    case 0x0D:  // CR
      t->x = 0;
      t->wrap_pending = false;
      break;
  }
}

static void align_test(Term *t) {
  Cell e = cell_blank(COLOR_DEFAULT);
  e.ch = 'E';
  for (int y = 0; y < t->rows; y++) line_fill(&t->screen[y], 0, t->cols, e);
  term_move_to(t, 0, 0);
}

void term_esc(void *user, const char *inter, uint8_t final) {
  Term *t = user;
  if (inter[0] == '(') {
    t->g0_graphics = (final == '0');
    return;
  }
  if (inter[0] == '#' && final == '8') {
    align_test(t);
    return;
  }
  if (inter[0]) return;  // other character sets and unknown sequences
  switch (final) {
    case '7': term_save_cursor(t); break;
    case '8': term_restore_cursor(t); break;
    case 'D': term_index(t); break;
    case 'E': term_index(t); t->x = 0; break;
    case 'M': term_reverse_index(t); break;
    case 'H': t->tabs[t->x] = true; break;
    case 'c': term_full_reset(t); break;
    case '=': t->modes.app_keypad = true; break;
    case '>': t->modes.app_keypad = false; break;
  }
}
