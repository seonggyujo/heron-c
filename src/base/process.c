// CreateProcess with a pipe for stdout. Only the pipe and the NUL device
// are inherited (a handle list), so the child never holds a pty pipe.
#include "base/process.h"

#include <stdlib.h>
#include <windows.h>

#include "base/log.h"
#include "base/mem.h"
#include "base/utf.h"

static HANDLE open_nul(void) {
  SECURITY_ATTRIBUTES sa = {sizeof sa, NULL, TRUE};
  return CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING,
                     0, NULL);
}

static bool start(const ProcessRun *run, HANDLE out, HANDLE nul, PROCESS_INFORMATION *pi) {
  SIZE_T size = 0;
  InitializeProcThreadAttributeList(NULL, 1, 0, &size);
  LPPROC_THREAD_ATTRIBUTE_LIST list = xmalloc(size);
  HANDLE inherit[2] = {out, nul};
  bool ok = InitializeProcThreadAttributeList(list, 1, 0, &size) &&
            UpdateProcThreadAttribute(list, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherit, sizeof inherit, NULL, NULL);
  STARTUPINFOEXW si;
  ZeroMemory(&si, sizeof si);
  si.StartupInfo.cb = sizeof si;
  si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
  si.StartupInfo.hStdInput = nul;
  si.StartupInfo.hStdOutput = out;
  si.StartupInfo.hStdError = nul;
  si.lpAttributeList = list;
  wchar_t *cmd = utf8_to_wide(run->command_line, -1);
  wchar_t *cwd = run->cwd ? utf8_to_wide(run->cwd, -1) : NULL;
  wchar_t *env = run->env_count ? env_block_with(run->env, run->env_count) : NULL;
  DWORD flags = CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT;
  ok = ok && CreateProcessW(NULL, cmd, NULL, NULL, TRUE, flags, env, cwd, &si.StartupInfo, pi);
  if (!ok) log_debug("process", "cannot start %s: %lu", run->command_line, GetLastError());
  DeleteProcThreadAttributeList(list);
  free(list);
  free(cmd);
  free(cwd);
  free(env);
  return ok;
}

// Reads until the program ends and its output is drained, or time runs out.
static bool collect(HANDLE read, HANDLE process, unsigned timeout_ms, Buf *out) {
  ULONGLONG deadline = GetTickCount64() + timeout_ms;
  char chunk[4096];
  for (;;) {
    DWORD ready = 0, got = 0;
    if (!PeekNamedPipe(read, NULL, 0, NULL, &ready, NULL)) return true;  // the writer closed: done
    if (ready && ReadFile(read, chunk, sizeof chunk, &got, NULL) && got) {
      buf_append(out, chunk, got);
      continue;
    }
    if (GetTickCount64() > deadline) return false;
    WaitForSingleObject(process, 15);
  }
}

int process_run(const ProcessRun *run, Buf *out) {
  SECURITY_ATTRIBUTES sa = {sizeof sa, NULL, TRUE};
  HANDLE read = NULL, write = NULL, nul = open_nul();
  if (nul == INVALID_HANDLE_VALUE || !CreatePipe(&read, &write, &sa, 0)) {
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    return -1;
  }
  SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
  PROCESS_INFORMATION pi = {0};
  bool started = start(run, write, nul, &pi);
  CloseHandle(write);
  CloseHandle(nul);
  int code = -1;
  if (started) {
    if (collect(read, pi.hProcess, run->timeout_ms, out)) {
      WaitForSingleObject(pi.hProcess, run->timeout_ms);
      DWORD exit_code;
      if (GetExitCodeProcess(pi.hProcess, &exit_code) && exit_code != STILL_ACTIVE) code = (int)exit_code;
    } else {
      log_warn("process", "%s ran too long, killed", run->command_line);
      TerminateProcess(pi.hProcess, 1);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }
  CloseHandle(read);
  return code;
}
