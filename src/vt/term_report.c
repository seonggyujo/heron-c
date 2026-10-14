// CSI sequences that ask the terminal something. Answers go to the replies
// buffer, which the pane writes back to the program.
#include "vt/term_internal.h"

static void device_attributes(Term *t, char prefix, const VtParams *p) {
  if (vt_param(p, 0, 0) != 0) return;
  if (prefix == 0) buf_append_str(&t->replies, "\x1b[?1;2c");  // VT100 with AVO, as xterm.js
  if (prefix == '>') buf_append_str(&t->replies, "\x1b[>0;276;0c");
}

static void device_status(Term *t, const VtParams *p) {
  switch (vt_param(p, 0, 0)) {
    case 5:
      buf_append_str(&t->replies, "\x1b[0n");
      break;
    case 6: {
      int row = t->y - (t->origin ? t->top : 0);
      buf_printf(&t->replies, "\x1b[%d;%dR", row + 1, t->x + 1);
      break;
    }
  }
}

static void window_report(Term *t, const VtParams *p) {
  if (vt_param(p, 0, 0) == 18) buf_printf(&t->replies, "\x1b[8;%d;%dt", t->rows, t->cols);
}

static void mode_report(Term *t, char prefix, const VtParams *p) {
  int mode = vt_param(p, 0, 0);
  int state = term_mode_state(t, prefix, mode);
  if (prefix == '?') {
    buf_printf(&t->replies, "\x1b[?%d;%d$y", mode, state);
  } else {
    buf_printf(&t->replies, "\x1b[%d;%d$y", mode, state);
  }
}

void term_report(Term *t, char prefix, const VtParams *p, uint8_t final) {
  switch (final) {
    case 'c':
      device_attributes(t, prefix, p);
      break;
    case 'n':
      if (prefix == 0) device_status(t, p);
      break;
    case 't':
      if (prefix == 0) window_report(t, p);
      break;
    case 'p':  // DECRQM, reached through the '$' intermediate
      mode_report(t, prefix, p);
      break;
  }
}
