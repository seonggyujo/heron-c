// Keyboard, paste, copy and focus reports for a pane.
#include "pane/pane_input.h"

#include <stdlib.h>

#include "input/paste.h"

void pane_send(Pane *p, const char *data, size_t len) {
  if (!pane_running(p) || len == 0) return;
  term_view_reset(p->term);
  selection_clear(&p->selection);
  pty_write(p->pty, data, len);
}

bool pane_key(Pane *p, unsigned vk, KeyMods mods) {
  char out[16];
  int n = keys_encode(vk, mods, term_modes(p->term).app_cursor, out);
  if (n == 0) return false;
  pane_send(p, out, (size_t)n);
  return true;
}

void pane_char(Pane *p, uint32_t ch, bool alt) {
  char out[16];
  int n = keys_encode_char(ch, alt, out);
  pane_send(p, out, (size_t)n);
}

void pane_paste(Pane *p, const char *utf8) {
  size_t n;
  char *bytes = paste_prepare(utf8, term_modes(p->term).bracketed_paste, &n);
  pane_send(p, bytes, n);
  free(bytes);
}

char *pane_take_selection(Pane *p) {
  char *text = selection_text(&p->selection, p->term);
  selection_clear(&p->selection);
  return text;
}

bool pane_has_selection(const Pane *p) { return p->selection.active; }

void pane_focus_changed(Pane *p, bool focused) {
  if (pane_running(p) && term_modes(p->term).focus_events) {
    pty_write(p->pty, focused ? "\x1b[I" : "\x1b[O", 3);
  }
}
