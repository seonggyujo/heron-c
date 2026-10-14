// One terminal pane: a shell in a pty, its terminal screen and selection, and
// what its header shows. pane_list.c owns and places panes; pane_input.c and
// pane_mouse.c take the user's input; pane_paint.c draws a pane.
#pragma once

#include <stdbool.h>
#include <windows.h>

#include "base/box.h"
#include "base/buf.h"
#include "pty/pty.h"
#include "pty/shell.h"
#include "render/mono_font.h"
#include "vt/selection.h"
#include "vt/term.h"

// Pane sizes in pixels, already scaled for the screen.
typedef struct {
  int header_h;
  int pad_left, pad_top, pad_right, pad_bottom;  // between the frame and the cells
  int radius;
  int close_size, close_right;  // the header's close button
} PaneMetrics;

// What a new pane starts: shell in cwd (NULL: home) with env added, and a
// command typed at the shell's first prompt (NULL: none), as if the user
// typed it.
typedef struct {
  ShellKind shell;
  const wchar_t *cwd;
  const EnvVar *env;
  int env_count;
  const char *command;
} PaneStart;

// Where the pty messages go: wParam is the pane id.
typedef struct {
  HWND hwnd;
  UINT msg_output, msg_exit;
} PaneHost;

typedef struct {
  int id;
  ShellKind shell;
  Term *term;
  Pty *pty;   // NULL when the shell failed to start
  Buf inbox;  // output taken from the pty, reused between drains
  Selection selection;
  Box frame;  // the whole pane, header included
  Box body;   // where cell (0, 0) starts
  int cell_w, cell_h;
  bool exited;
  DWORD exit_code;
  char *start_command;             // typed at the first prompt, then freed
  char *start_cwd;                 // the folder it was opened in (UTF-8), NULL: home
  ULONGLONG started_at, ended_at;  // GetTickCount64
  ULONGLONG output_at;             // last output that flashed the border
  bool selecting;                  // the left button is dragging a selection
  int buttons_down;                // buttons held while the program tracks the mouse
  int last_col, last_row;          // last cell reported to a tracking program
  bool has_hover_link;             // the mouse is over a link: hover_link is underlined
  TermLink hover_link;
  // Animation (ui/pane_motion.c): where the pane was drawn when its box
  // last changed, and when that was, when it appeared and when it closed.
  // Times are GetTickCount64, 0 for none.
  Box moved_from;
  ULONGLONG moved_at, opened_at, closed_at;
} Pane;

// How many cells fit in a pane placed in frame.
void pane_grid_size(Box frame, const PaneMetrics *m, const MonoFont *font, int *cols, int *rows);

Pane *pane_open(int id, const PaneStart *start, int cols, int rows, const PaneHost *host);
void pane_free(Pane *p);
// The folder the shell last reported, else the one it was opened in; NULL
// when neither is known (opened in home, no prompt yet).
const char *pane_path(const Pane *p);

// Feeds waiting output to the terminal and writes its replies back.
// Returns the TERM_EVENT_* bits that output raised.
unsigned pane_drain(Pane *p);
void pane_set_exited(Pane *p, DWORD code);
bool pane_running(const Pane *p);
void pane_kill(Pane *p);

// Puts the pane in frame and fits the terminal and pty to it.
void pane_place(Pane *p, Box frame, const PaneMetrics *m, const MonoFont *font);
