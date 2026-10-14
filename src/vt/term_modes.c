// DEC private and ANSI modes (CSI ? n h / l, CSI n h / l), the saved cursor
// and switching to the alternate screen.
#include "vt/line.h"
#include "vt/term_internal.h"

static SavedCursor *saved_slot(Term *t) { return t->alt_active ? &t->saved_alt : &t->saved_main; }

void term_save_cursor(Term *t) {
  SavedCursor *s = saved_slot(t);
  s->x = t->x;
  s->y = t->y;
  s->pen = t->pen;
  s->origin = t->origin;
  s->g0_graphics = t->g0_graphics;
}

void term_restore_cursor(Term *t) {
  SavedCursor *s = saved_slot(t);
  t->x = s->x < t->cols ? s->x : t->cols - 1;
  t->y = s->y < t->rows ? s->y : t->rows - 1;
  t->pen = s->pen;
  t->origin = s->origin;
  t->g0_graphics = s->g0_graphics;
  t->wrap_pending = false;
}

// 1049 saves the cursor on the main screen before switching and restores it
// after coming back; 47 and 1047 only switch. The alternate screen always
// starts empty and has no history.
static void use_alt_screen(Term *t, bool alt, bool keep_cursor) {
  if (alt == t->alt_active) return;
  if (alt) {
    if (keep_cursor) term_save_cursor(t);
    t->alt_active = true;
    t->screen = t->alt_lines;
    for (int y = 0; y < t->rows; y++) {
      line_fill(&t->screen[y], 0, t->cols, term_blank(t));
      t->screen[y].wrapped = false;
    }
    t->view_offset = 0;
  } else {
    t->alt_active = false;
    t->screen = t->main_lines;
    if (keep_cursor) term_restore_cursor(t);
  }
  t->modes.alt_screen = alt;
}

static void set_dec_mode(Term *t, int mode, bool on) {
  switch (mode) {
    case 1: t->modes.app_cursor = on; break;
    case 6: t->origin = on; term_move_to(t, 0, 0); break;
    case 7: t->autowrap = on; break;
    case 25: t->modes.cursor_visible = on; break;
    case 47:
    case 1047: use_alt_screen(t, on, false); break;
    case 1049: use_alt_screen(t, on, true); break;
    case 9:
    case 1000:
    case 1002:
    case 1003: t->modes.mouse = on ? (mode == 9 ? 1000 : mode) : 0; break;
    case 1004: t->modes.focus_events = on; break;
    case 1006: t->modes.mouse_sgr = on; break;
    case 2004: t->modes.bracketed_paste = on; break;
    case 2026: t->modes.synchronized = on; break;
  }
}

void term_set_modes(Term *t, char prefix, const VtParams *p, bool on) {
  for (int i = 0; i < p->n; i++) {
    int mode = vt_param(p, i, 0);
    if (prefix == '?') {
      set_dec_mode(t, mode, on);
    } else if (mode == 4) {
      t->insert = on;
    } else if (mode == 20) {
      t->newline_mode = on;
    }
  }
}

static int state(bool on) { return on ? 1 : 2; }

int term_mode_state(const Term *t, char prefix, int mode) {
  if (prefix != '?') {
    if (mode == 4) return state(t->insert);
    if (mode == 20) return state(t->newline_mode);
    return 0;
  }
  switch (mode) {
    case 1: return state(t->modes.app_cursor);
    case 6: return state(t->origin);
    case 7: return state(t->autowrap);
    case 25: return state(t->modes.cursor_visible);
    case 47:
    case 1047:
    case 1049: return state(t->alt_active);
    case 1000:
    case 1002:
    case 1003: return state(t->modes.mouse == mode);
    case 1004: return state(t->modes.focus_events);
    case 1006: return state(t->modes.mouse_sgr);
    case 2004: return state(t->modes.bracketed_paste);
    case 2026: return state(t->modes.synchronized);
    default: return 0;
  }
}
