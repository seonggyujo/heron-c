// Starting a program in a pseudoconsole, and the window-thread calls.
#include <stdlib.h>
#include <string.h>

#include "base/log.h"
#include "base/mem.h"
#include "pty/pty_internal.h"

void pty_init_process(void) { SetConsoleCtrlHandler(NULL, FALSE); }

static bool spawn(Pty *p, const PtyOptions *o, HPCON console) {
  STARTUPINFOEXW si;
  memset(&si, 0, sizeof si);
  si.StartupInfo.cb = sizeof si;
  // Without explicit invalid handles the program can inherit our own
  // redirected stdio instead of the console (as portable-pty notes).
  si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
  si.StartupInfo.hStdInput = si.StartupInfo.hStdOutput = si.StartupInfo.hStdError =
      INVALID_HANDLE_VALUE;
  SIZE_T size = 0;
  InitializeProcThreadAttributeList(NULL, 1, 0, &size);
  si.lpAttributeList = xmalloc(size);
  bool ok = InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &size) &&
            UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                      console, sizeof console, NULL, NULL);
  PROCESS_INFORMATION pi;
  wchar_t *command = _wcsdup(o->command_line);  // CreateProcessW may write to it
  DWORD flags = EXTENDED_STARTUPINFO_PRESENT | (o->environment ? CREATE_UNICODE_ENVIRONMENT : 0);
  ok = ok && command &&
       CreateProcessW(NULL, command, NULL, NULL, FALSE, flags, (void *)o->environment, o->cwd,
                      &si.StartupInfo, &pi);
  if (!ok) log_error("pty", "CreateProcess failed: error %lu", GetLastError());
  DeleteProcThreadAttributeList(si.lpAttributeList);
  free(si.lpAttributeList);
  free(command);
  if (!ok) return false;
  CloseHandle(pi.hThread);
  p->process = pi.hProcess;
  p->pid = pi.dwProcessId;
  return true;
}

Pty *pty_start(const PtyOptions *o) {
  HANDLE in_read = NULL, in_write = NULL, out_read = NULL, out_write = NULL;
  HPCON console = NULL;
  if (!CreatePipe(&in_read, &in_write, NULL, 0) || !CreatePipe(&out_read, &out_write, NULL, 0)) {
    log_error("pty", "CreatePipe failed: error %lu", GetLastError());
    goto fail;
  }
  COORD size = {(SHORT)o->cols, (SHORT)o->rows};
  HRESULT hr = CreatePseudoConsole(size, in_read, out_write, 0, &console);
  // The console keeps its own copies of its pipe ends.
  CloseHandle(in_read);
  CloseHandle(out_write);
  in_read = out_write = NULL;
  if (FAILED(hr)) {
    log_error("pty", "CreatePseudoConsole failed: 0x%08lx", (unsigned long)hr);
    goto fail;
  }
  Pty *p = xcalloc(1, sizeof *p);
  if (!spawn(p, o, console)) {
    free(p);
    goto fail;
  }
  InitializeCriticalSection(&p->lock);
  p->console = console;
  p->input = in_write;
  p->output = out_read;
  p->drained = CreateEventW(NULL, FALSE, FALSE, NULL);
  p->kill_event = CreateEventW(NULL, TRUE, FALSE, NULL);
  p->notify = o->notify;
  p->msg_output = o->msg_output;
  p->msg_exit = o->msg_exit;
  p->tag = o->tag;
  p->reader = CreateThread(NULL, 0, pty_reader_main, p, 0, NULL);
  p->waiter = CreateThread(NULL, 0, pty_waiter_main, p, 0, NULL);
  log_debug("pty", "started pid %lu, %dx%d", p->pid, o->cols, o->rows);
  return p;

fail:
  if (console) ClosePseudoConsole(console);
  HANDLE handles[] = {in_read, in_write, out_read, out_write};
  for (int i = 0; i < 4; i++) {
    if (handles[i]) CloseHandle(handles[i]);
  }
  return NULL;
}

void pty_take_output(Pty *p, Buf *out) {
  EnterCriticalSection(&p->lock);
  buf_append(out, p->pending.data, p->pending.len);
  buf_clear(&p->pending);
  p->notified = false;
  LeaveCriticalSection(&p->lock);
  SetEvent(p->drained);
}

void pty_write(Pty *p, const void *data, size_t len) {
  const char *at = data;
  while (len > 0) {
    DWORD written = 0;
    if (!WriteFile(p->input, at, (DWORD)len, &written, NULL)) return;
    at += written;
    len -= written;
  }
}

void pty_resize(Pty *p, int cols, int rows) {
  EnterCriticalSection(&p->lock);
  if (p->console) ResizePseudoConsole(p->console, (COORD){(SHORT)cols, (SHORT)rows});
  LeaveCriticalSection(&p->lock);
  log_debug("pty", "pid %lu resized to %dx%d", p->pid, cols, rows);
}

void pty_kill(Pty *p) {
  InterlockedExchange(&p->discard, 1);
  SetEvent(p->drained);
  SetEvent(p->kill_event);
}

void pty_free(Pty *p) {
  WaitForSingleObject(p->waiter, INFINITE);
  HANDLE handles[] = {p->reader, p->waiter, p->process, p->input, p->output, p->drained,
                      p->kill_event};
  for (size_t i = 0; i < sizeof handles / sizeof handles[0]; i++) CloseHandle(handles[i]);
  DeleteCriticalSection(&p->lock);
  buf_free(&p->pending);
  free(p);
}
