// Finding the installed shells and naming them.
#include "pty/shell.h"

#include <string.h>
#include <windows.h>

#include "base/log.h"

static wchar_t g_programs[SHELL_COUNT][MAX_PATH];

static const char *const NAMES[SHELL_COUNT] = {"pwsh", "PowerShell", "cmd", "Git Bash"};
static const char *const IDS[SHELL_COUNT] = {"pwsh", "powershell", "cmd", "gitbash"};

static bool file_exists(const wchar_t *path) {
  DWORD a = GetFileAttributesW(path);
  return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool find_in_path(const wchar_t *exe, wchar_t *out) {
  return SearchPathW(NULL, exe, NULL, MAX_PATH, out, NULL) > 0;
}

static bool use_if_exists(const wchar_t *path, wchar_t *out) {
  if (!file_exists(path)) return false;
  wcscpy(out, path);
  return true;
}

// git.exe lives in <root>\cmd and bash.exe in <root>\bin.
static bool find_git_bash(wchar_t *out) {
  wchar_t git[MAX_PATH];
  if (find_in_path(L"git.exe", git)) {
    wchar_t *slash = wcsrchr(git, L'\\');
    if (slash) *slash = 0;
    slash = wcsrchr(git, L'\\');
    if (slash && wcslen(git) + 14 < MAX_PATH) {
      wcscpy(slash, L"\\bin\\bash.exe");
      if (use_if_exists(git, out)) return true;
    }
  }
  return use_if_exists(L"C:\\Program Files\\Git\\bin\\bash.exe", out);
}

void shell_detect(void) {
  wchar_t *p = g_programs[SHELL_PWSH];
  if (!find_in_path(L"pwsh.exe", p)) use_if_exists(L"C:\\Program Files\\PowerShell\\7\\pwsh.exe", p);
  find_in_path(L"powershell.exe", g_programs[SHELL_POWERSHELL]);
  DWORD n = GetEnvironmentVariableW(L"ComSpec", g_programs[SHELL_CMD], MAX_PATH);
  if (n == 0 || n >= MAX_PATH || !file_exists(g_programs[SHELL_CMD])) {
    g_programs[SHELL_CMD][0] = 0;
    find_in_path(L"cmd.exe", g_programs[SHELL_CMD]);
  }
  find_git_bash(g_programs[SHELL_GIT_BASH]);
  for (int k = 0; k < SHELL_COUNT; k++) {
    log_debug("shell", "%s: %s", NAMES[k], g_programs[k][0] ? "found" : "not installed");
  }
}

bool shell_installed(ShellKind kind) { return g_programs[kind][0] != 0; }

ShellKind shell_default(void) {
  for (int k = 0; k < SHELL_COUNT; k++) {
    if (shell_installed((ShellKind)k)) return (ShellKind)k;
  }
  return SHELL_POWERSHELL;
}

const char *shell_name(ShellKind kind) { return NAMES[kind]; }
const char *shell_id(ShellKind kind) { return IDS[kind]; }

bool shell_from_id(const char *id, ShellKind *out) {
  for (int k = 0; k < SHELL_COUNT; k++) {
    if (strcmp(id, IDS[k]) == 0) {
      *out = (ShellKind)k;
      return true;
    }
  }
  return false;
}

const wchar_t *shell_program(ShellKind kind) { return shell_installed(kind) ? g_programs[kind] : NULL; }
