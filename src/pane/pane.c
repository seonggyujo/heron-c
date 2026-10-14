// Opening a pane's shell, feeding its output to the terminal, and fitting
// the terminal to the pane.
#include "pane/pane.h"

#include <stdlib.h>

#include "base/log.h"
#include "base/mem.h"
#include "base/utf.h"

#define HISTORY_LINES 5000

static Box body_box(Box frame, const PaneMetrics *m) {
  Box b = {frame.x + m->pad_left, frame.y + m->header_h + m->pad_top,
           frame.w - m->pad_left - m->pad_right, frame.h - m->header_h - m->pad_top - m->pad_bottom};
  return b;
}

void pane_grid_size(Box frame, const PaneMetrics *m, const MonoFont *font, int *cols, int *rows) {
  Box body = body_box(frame, m);
  int c = body.w / mono_font_cell_w(font), r = body.h / mono_font_cell_h(font);
  *cols = c > 2 ? c : 2;
  *rows = r > 1 ? r : 1;
}

Pane *pane_open(int id, const PaneStart *start, int cols, int rows, const PaneHost *host) {
  Pane *p = xcalloc(1, sizeof *p);
  ShellKind shell = start->shell;
  p->id = id;
  p->shell = shell;
  if (start->command) p->start_command = _strdup(start->command);
  if (start->cwd) p->start_cwd = wide_to_utf8(start->cwd, -1, NULL);
  p->term = term_new(cols, rows, HISTORY_LINES);
  p->started_at = GetTickCount64();
  p->last_col = p->last_row = -1;
  ShellLaunch launch;
  if (shell_launch(shell, start->cwd, start->env, start->env_count, &launch)) {
    PtyOptions o = {launch.command_line, launch.cwd, launch.environment, cols, rows,
                    host->hwnd, host->msg_output, host->msg_exit, (WPARAM)id};
    p->pty = pty_start(&o);
    shell_launch_free(&launch);
  }
  if (!p->pty) {
    p->exited = true;
    p->ended_at = p->started_at;
    log_warn("pane", "pane %d: %s failed to start", id, shell_name(shell));
  } else {
    log_info("pane", "pane %d: %s, %dx%d", id, shell_name(shell), cols, rows);
  }
  return p;
}

void pane_free(Pane *p) {
  if (!p) return;
  if (p->pty) pty_free(p->pty);
  term_free(p->term);
  buf_free(&p->inbox);
  free(p->start_command);
  free(p->start_cwd);
  free(p);
}

const char *pane_path(const Pane *p) {
  const char *cwd = term_cwd(p->term);
  return cwd[0] ? cwd : p->start_cwd;
}

unsigned pane_drain(Pane *p) {
  if (!p->pty) return 0;
  pty_take_output(p->pty, &p->inbox);
  log_bytes(LOG_TRACE, "pane", "output", p->inbox.data, p->inbox.len);
  term_feed(p->term, (const uint8_t *)p->inbox.data, p->inbox.len);
  buf_clear(&p->inbox);
  Buf *replies = term_replies(p->term);
  if (replies->len && !p->exited) pty_write(p->pty, replies->data, replies->len);
  buf_clear(replies);
  return term_take_events(p->term);
}

void pane_set_exited(Pane *p, DWORD code) {
  pane_drain(p);  // the last output arrives before the exit message
  p->exited = true;
  p->exit_code = code;
  p->ended_at = GetTickCount64();
  log_info("pane", "pane %d: exit %lu", p->id, code);
}

bool pane_running(const Pane *p) { return p->pty && !p->exited; }

void pane_kill(Pane *p) {
  if (pane_running(p)) pty_kill(p->pty);
}

void pane_place(Pane *p, Box frame, const PaneMetrics *m, const MonoFont *font) {
  p->frame = frame;
  p->body = body_box(frame, m);
  p->cell_w = mono_font_cell_w(font);
  p->cell_h = mono_font_cell_h(font);
  int cols, rows;
  pane_grid_size(frame, m, font, &cols, &rows);
  if (cols == term_cols(p->term) && rows == term_rows(p->term)) return;
  term_resize(p->term, cols, rows);
  if (pane_running(p)) pty_resize(p->pty, cols, rows);
}
