// A terminal screen: feed it the bytes a program prints, then read back the
// cells, cursor, title and modes. No Win32 and no drawing here: the pane
// connects a Term to a pty and the renderer draws what it holds.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "base/buf.h"
#include "vt/cell.h"

typedef struct Term Term;

// Modes the keyboard, mouse and renderer need to know about.
typedef struct {
  bool app_cursor;  // DECCKM: arrow keys send ESC O x
  bool app_keypad;
  bool bracketed_paste;
  bool focus_events;
  bool alt_screen;
  bool cursor_visible;
  bool synchronized;  // mode 2026: the program is mid-frame, hold the paint
  bool mouse_sgr;
  int mouse;         // 0, or 1000 (clicks), 1002 (drags), 1003 (all motion)
  int cursor_style;  // DECSCUSR: 0-2 block, 3-4 underline, 5-6 bar
} TermModes;

// PROMPT: the shell reported its folder, as the shells heron-c starts do
// right before each prompt (also when the folder did not change).
enum { TERM_EVENT_TITLE = 1, TERM_EVENT_CWD = 2, TERM_EVENT_BELL = 4, TERM_EVENT_PROMPT = 8 };

Term *term_new(int cols, int rows, int history_lines);
void term_free(Term *t);
void term_feed(Term *t, const uint8_t *data, size_t len);
void term_resize(Term *t, int cols, int rows);

int term_cols(const Term *t);
int term_rows(const Term *t);
TermModes term_modes(const Term *t);
void term_cursor(const Term *t, int *x, int *y);
const char *term_title(const Term *t);  // UTF-8, may be empty
const char *term_cwd(const Term *t);    // Windows path, may be empty

// Returns the TERM_EVENT_* bits raised since the last call and clears them.
unsigned term_take_events(Term *t);

// Answers to queries (cursor position, device attributes) waiting to be
// written back to the program. The caller writes them out and clears it.
Buf *term_replies(Term *t);

// Looking back through history. Offset 0 shows the live screen; larger
// offsets show older lines. The view stays put while new output arrives.
int term_history_size(const Term *t);
int term_view_offset(const Term *t);
void term_scroll_view(Term *t, int lines);  // positive goes back in time
void term_view_reset(Term *t);
const Line *term_view_line(const Term *t, int y);  // row y of what is shown

// Stable ids for rows, so a selection stays on its text while history
// scrolls. term_line_by_id returns NULL once the line has been dropped.
uint64_t term_view_row_id(const Term *t, int y);
const Line *term_line_by_id(const Term *t, uint64_t id);

// A link under the mouse: an OSC 8 hyperlink, or a plain http(s) URL in
// the text (also one that wraps onto the next lines). The cells run from
// (row0, col0) to (row1, col1), both inclusive.
typedef struct {
  char url[2048];
  uint64_t row0, row1;
  int col0, col1;
} TermLink;

bool term_link_at(const Term *t, uint64_t row, int col, TermLink *out);
// The URI of an OSC 8 link id, NULL for none.
const char *term_link_uri(const Term *t, uint16_t id);
