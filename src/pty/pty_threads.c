// The reader thread (console output into the pending buffer) and the waiter
// thread (program exit, console close, exit message).
#include "base/log.h"
#include "pty/pty_internal.h"

// The reader pauses above this much unread output, so a flood cannot use
// unbounded memory; the console then blocks the program until we catch up.
#define PENDING_LIMIT (4u << 20)

static size_t pending_len(Pty *p) {
  EnterCriticalSection(&p->lock);
  size_t n = p->pending.len;
  LeaveCriticalSection(&p->lock);
  return n;
}

DWORD WINAPI pty_reader_main(void *arg) {
  Pty *p = arg;
  char chunk[65536];
  for (;;) {
    DWORD n = 0;
    if (!ReadFile(p->output, chunk, sizeof chunk, &n, NULL) || n == 0) break;
    if (p->discard) continue;
    log_trace("pty", "pid %lu: read %lu bytes", p->pid, n);
    EnterCriticalSection(&p->lock);
    buf_append(&p->pending, chunk, n);
    bool post = !p->notified;
    p->notified = true;
    LeaveCriticalSection(&p->lock);
    if (post) PostMessageW(p->notify, p->msg_output, p->tag, 0);
    while (!p->discard && pending_len(p) > PENDING_LIMIT) WaitForSingleObject(p->drained, 100);
  }
  return 0;
}

void pty_close_console(Pty *p) {
  EnterCriticalSection(&p->lock);
  HPCON console = p->console;
  p->console = NULL;
  LeaveCriticalSection(&p->lock);
  if (console) ClosePseudoConsole(console);
}

DWORD WINAPI pty_waiter_main(void *arg) {
  Pty *p = arg;
  HANDLE waits[2] = {p->process, p->kill_event};
  if (WaitForMultipleObjects(2, waits, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
    // Closing the console sends the programs in it a close event; force
    // the shell down if it ignores that.
    pty_close_console(p);
    if (WaitForSingleObject(p->process, 3000) == WAIT_TIMEOUT) TerminateProcess(p->process, 1);
  }
  DWORD code = 0;
  GetExitCodeProcess(p->process, &code);
  // Programs the shell left running still hold the console; closing it
  // ends them and lets the reader reach the end of the output.
  pty_close_console(p);
  WaitForSingleObject(p->reader, INFINITE);
  log_debug("pty", "pid %lu exited with %lu", p->pid, code);
  PostMessageW(p->notify, p->msg_exit, p->tag, (LPARAM)code);
  return 0;
}
