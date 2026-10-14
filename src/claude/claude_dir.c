// Finding Claude Code's config folder.
#include "claude/claude_dir.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "base/utf.h"

static bool variable(const wchar_t *name, char *out, size_t cap) {
  wchar_t value[MAX_PATH];
  DWORD n = GetEnvironmentVariableW(name, value, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return false;
  char *utf8 = wide_to_utf8(value, -1, NULL);
  snprintf(out, cap, "%s", utf8);
  free(utf8);
  return true;
}

bool claude_config_dir(char *out, size_t cap) {
  if (variable(L"CLAUDE_CONFIG_DIR", out, cap)) return true;
  char home[MAX_PATH * 3];
  if (!variable(L"USERPROFILE", home, sizeof home)) return false;
  snprintf(out, cap, "%s\\.claude", home);
  return true;
}
