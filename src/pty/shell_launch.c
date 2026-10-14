// The command line, environment and folder that start a shell, with the hook
// that makes it report its folder (OSC 9;9, the sequence Windows Terminal
// uses) right before each prompt. The hook lives only in that shell session.
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "base/buf.h"
#include "base/mem.h"
#include "base/utf.h"
#include "pty/shell.h"

// Wraps whatever prompt function the profile defined. [Console]::Write keeps
// PSReadLine from counting the sequence as prompt text.
static const char POWERSHELL_HOOK[] =
    "$global:__heronPrompt = $function:prompt; function global:prompt { "
    "$l = $executionContext.SessionState.Path.CurrentLocation; "
    "if ($l.Provider.Name -eq 'FileSystem') { "
    "[Console]::Write(\"$([char]27)]9;9;$($l.ProviderPath)$([char]27)\\\") }; "
    "& $global:__heronPrompt }";

// Bash reports $PWD in MSYS form (/c/Users/...); vt/term_osc.c converts it.
static const wchar_t BASH_HOOK[] = L"printf '\\e]9;9;%s\\e\\\\' \"$PWD\"";

// cmd expands $e (ESC) and $P (drive and path) in PROMPT.
static const wchar_t CMD_HOOK[] = L"$e]9;9;$P$e\\";

// Quotes one argument the way CommandLineToArgvW reads it back: backslashes
// before a quote are doubled and the quote escaped.
static void append_arg(Buf *b, const char *arg) {
  buf_append(b, " \"", 2);
  int slashes = 0;
  for (const char *p = arg; *p; p++) {
    if (*p == '\\') {
      slashes++;
      continue;
    }
    int n = *p == '"' ? slashes * 2 + 1 : slashes;
    for (int i = 0; i < n; i++) buf_append(b, "\\", 1);
    buf_append(b, p, 1);
    slashes = 0;
  }
  for (int i = 0; i < slashes * 2; i++) buf_append(b, "\\", 1);
  buf_append(b, "\"", 1);
}

static wchar_t *command_line(ShellKind kind) {
  Buf b = {0};
  char *program = wide_to_utf8(shell_program(kind), -1, NULL);
  buf_printf(&b, "\"%s\"", program);
  free(program);
  switch (kind) {
    case SHELL_PWSH:
    case SHELL_POWERSHELL:
      buf_append_str(&b, " -NoLogo -NoExit -Command");
      append_arg(&b, POWERSHELL_HOOK);
      break;
    case SHELL_GIT_BASH:
      buf_append_str(&b, " --login -i");
      break;
    default:
      break;
  }
  wchar_t *w = utf8_to_wide(b.data, (ptrdiff_t)b.len);
  buf_free(&b);
  return w;
}

#define MAX_ENV 8

// The prompt hook of cmd and Git Bash, then env. The user's own prompt
// goes after ours so it still works.
static wchar_t *environment(ShellKind kind, const EnvVar *env, int env_count) {
  EnvVar vars[MAX_ENV];
  int count = 0;
  for (int i = 0; i < env_count && count < MAX_ENV - 1; i++) vars[count++] = env[i];
  if (kind != SHELL_CMD && kind != SHELL_GIT_BASH) return count ? env_block_with(vars, count) : NULL;
  const wchar_t *name = kind == SHELL_CMD ? L"PROMPT" : L"PROMPT_COMMAND";
  const wchar_t *hook = kind == SHELL_CMD ? CMD_HOOK : BASH_HOOK;
  wchar_t user[2048];
  DWORD n = GetEnvironmentVariableW(name, user, 2048);
  if (n >= 2048) n = 0;
  user[n] = 0;
  if (kind == SHELL_CMD && n == 0) wcscpy(user, L"$P$G");
  size_t len = wcslen(hook) + wcslen(user) + 3;
  wchar_t *value = xmalloc(len * sizeof(wchar_t));
  wcscpy(value, hook);
  if (kind == SHELL_GIT_BASH && user[0]) wcscat(value, L"; ");
  wcscat(value, user);
  vars[count++] = (EnvVar){name, value};
  wchar_t *block = env_block_with(vars, count);
  free(value);
  return block;
}

static wchar_t *start_folder(const wchar_t *cwd) {
  DWORD a = cwd ? GetFileAttributesW(cwd) : INVALID_FILE_ATTRIBUTES;
  if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY)) return _wcsdup(cwd);
  wchar_t home[MAX_PATH];
  DWORD n = GetEnvironmentVariableW(L"USERPROFILE", home, MAX_PATH);
  return n > 0 && n < MAX_PATH ? _wcsdup(home) : NULL;
}

bool shell_launch(ShellKind kind, const wchar_t *cwd, const EnvVar *env, int env_count, ShellLaunch *out) {
  if (!shell_installed(kind)) return false;
  out->command_line = command_line(kind);
  out->environment = environment(kind, env, env_count);
  out->cwd = start_folder(cwd);
  return true;
}

void shell_launch_free(ShellLaunch *l) {
  free(l->command_line);
  free(l->environment);
  free(l->cwd);
  l->command_line = l->environment = l->cwd = NULL;
}
