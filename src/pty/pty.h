// A program running in a Windows pseudoconsole (ConPTY). A reader thread
// collects its output and posts msg_output when new output is waiting; a
// waiter thread posts msg_exit with the exit code when it ends. All other
// calls belong on the window thread.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <windows.h>

#include "base/buf.h"

typedef struct Pty Pty;

typedef struct {
  const wchar_t *command_line;
  const wchar_t *cwd;          // NULL: the current folder
  const wchar_t *environment;  // UTF-16 environment block, NULL: inherit
  int cols, rows;
  HWND notify;
  UINT msg_output;  // wParam = tag
  UINT msg_exit;    // wParam = tag, lParam = exit code
  WPARAM tag;
} PtyOptions;

// Once per process, before the first pty_start: undoes an inherited "ignore
// Ctrl+C" state, which shells would inherit too (Windows Terminal does the
// same).
void pty_init_process(void);

// Starts the program. Returns NULL and logs why on failure.
Pty *pty_start(const PtyOptions *opt);

// Moves all waiting output to the end of out.
void pty_take_output(Pty *p, Buf *out);

void pty_write(Pty *p, const void *data, size_t len);
void pty_resize(Pty *p, int cols, int rows);

// Closes the console, which ends the program and everything it started.
// msg_exit still follows; output after this is dropped.
void pty_kill(Pty *p);

// Frees the pty. Call only after msg_exit arrived.
void pty_free(Pty *p);
