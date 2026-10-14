// The Term struct and the helpers its files share. Only vt/term*.c include
// this; everything else goes through vt/term.h.
#pragma once

#include "vt/parser.h"
#include "vt/scrollback.h"
#include "vt/term.h"

typedef struct {
  int x, y;
  Cell pen;
  bool origin, g0_graphics;
} SavedCursor;

struct Term {
  int cols, rows;
  Line *screen;  // main_lines or alt_lines
  Line *main_lines, *alt_lines;
  bool alt_active;
  Scrollback history;
  int view_offset;

  int x, y;
  bool wrap_pending;  // the last column was written; the next print wraps first
  Cell pen;           // colors and attributes for new characters
  SavedCursor saved_main, saved_alt;
  int top, bottom;  // scroll region, inclusive
  bool *tabs;

  bool autowrap, origin, insert, newline_mode, g0_graphics;
  TermModes modes;
  uint32_t last_char;  // for REP

  char title[512];
  char cwd[1024];
  char **links;  // OSC 8 URIs; a cell's link field is an index + 1
  int link_count, link_cap;
  unsigned events;
  Buf replies;
  VtParser parser;
};

// term_screen.c: cursor, scrolling, erasing and editing on the active screen.
Cell term_blank(const Term *t);
void term_scroll_up(Term *t, int top, int bottom, int n, bool to_history);
void term_scroll_down(Term *t, int top, int bottom, int n);
void term_index(Term *t);
void term_reverse_index(Term *t);
void term_move_to(Term *t, int x, int y);  // y counts from the region top in origin mode
void term_tab(Term *t, int n);             // n > 0 forward, n < 0 back
void term_reset_tabs(Term *t);

// term_edit.c: erasing, inserting and deleting.
void term_erase_in_line(Term *t, int mode);
void term_erase_in_display(Term *t, int mode);
void term_erase_chars(Term *t, int n);
void term_insert_chars(Term *t, int n);
void term_delete_chars(Term *t, int n);
void term_insert_lines(Term *t, int n);
void term_delete_lines(Term *t, int n);

// term_print.c: parser callbacks for characters, controls and ESC sequences.
void term_print(void *user, uint32_t cp);
void term_execute(void *user, uint8_t control);
void term_esc(void *user, const char *inter, uint8_t final);

// term_csi.c and term_report.c: CSI sequences, and the ones that answer.
void term_csi(void *user, char prefix, const VtParams *p, const char *inter, uint8_t final);
void term_report(Term *t, char prefix, const VtParams *p, uint8_t final);

// term_sgr.c
void term_sgr(Term *t, const VtParams *p);

// term_modes.c: DEC and ANSI modes, saved cursor, alternate screen.
void term_set_modes(Term *t, char prefix, const VtParams *p, bool on);
int term_mode_state(const Term *t, char prefix, int mode);  // DECRQM: 0 unknown, 1 set, 2 reset
void term_save_cursor(Term *t);
void term_restore_cursor(Term *t);

// term_reset.c
void term_soft_reset(Term *t);
void term_full_reset(Term *t);

// term_osc.c
void term_osc(void *user, const char *data, size_t len);

// term_hyperlinks.c: OSC 8 "8;params;uri" starts (or with an empty uri ends)
// a hyperlink on the characters that follow.
void term_hyperlink(Term *t, const char *text, size_t len);
void term_free_links(Term *t);
