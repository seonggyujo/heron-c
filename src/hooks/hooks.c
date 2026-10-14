// settings.json read and written for the hooks. The file is written back
// atomically after a copy of the old one is kept beside it; a file that is
// not a JSON object is never written over.
#include "hooks/hooks.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "base/file.h"
#include "base/log.h"
#include "base/process.h"
#include "base/utf.h"
#include "claude/claude_dir.h"
#include "claude/claude_version.h"

#define BACKUP_SUFFIX ".heron-c-backup"
#define PATH_MAX_BYTES (MAX_PATH * 3)

static const ClaudeVersion MINIMUM = {2, 1, 139};

static HookResult result(HookError error) {
  HookResult r = {error, ""};
  return r;
}

static bool settings_path(char *out, size_t cap) {
  char dir[PATH_MAX_BYTES];
  if (!claude_config_dir(dir, sizeof dir)) return false;
  file_join(out, cap, dir, "settings.json");
  return true;
}

// The parsed file; an empty object when there is none yet.
static HookError read_settings(Json **out) {
  char path[PATH_MAX_BYTES];
  if (!settings_path(path, sizeof path)) return HOOKS_IO;
  size_t len;
  char *text = file_read(path, &len);
  if (!text) {
    if (file_exists(path)) return HOOKS_IO;
    *out = json_new(JSON_OBJECT);
    return HOOKS_OK;
  }
  *out = json_parse(text, len);
  free(text);
  if (json_is(*out, JSON_OBJECT)) return HOOKS_OK;
  json_free(*out);
  return HOOKS_BAD_SETTINGS;
}

static HookError write_settings(const Json *settings) {
  char path[PATH_MAX_BYTES], dir[PATH_MAX_BYTES], backup[PATH_MAX_BYTES + 32];
  if (!settings_path(path, sizeof path) || !claude_config_dir(dir, sizeof dir) || !file_make_dirs(dir)) return HOOKS_IO;
  snprintf(backup, sizeof backup, "%s%s", path, BACKUP_SUFFIX);
  if (file_exists(path) && !file_copy(path, backup)) return HOOKS_IO;
  Buf text = {0};
  json_write(settings, &text, true);  // two-space JSON, as Claude Code writes it
  buf_append(&text, "\n", 1);
  bool ok = file_write_atomic(path, text.data, text.len);
  buf_free(&text);
  return ok ? HOOKS_OK : HOOKS_IO;
}

static bool current_exe(char *out, size_t cap) {
  wchar_t w[MAX_PATH];
  DWORD n = GetModuleFileNameW(NULL, w, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return false;
  char *utf8 = wide_to_utf8(w, -1, NULL);
  snprintf(out, cap, "%s", utf8);
  free(utf8);
  return true;
}

// Through cmd: `claude` may be a .cmd shim (npm install).
static HookResult check_version(void) {
  Buf out = {0};
  ProcessRun run = {"cmd /D /C claude --version", NULL, NULL, 0, 20000};
  int code = process_run(&run, &out);
  buf_append(&out, "", 1);
  ClaudeVersion v;
  bool found = code == 0 && claude_version_parse(out.data, &v);
  buf_free(&out);
  if (!found) return result(HOOKS_NO_CLAUDE);
  if (claude_version_compare(v, MINIMUM) >= 0) return result(HOOKS_OK);
  HookResult r = result(HOOKS_CLAUDE_OLD);
  snprintf(r.version, sizeof r.version, "%u.%u.%u", v.major, v.minor, v.patch);
  return r;
}

HookResult hooks_presence(HookPresence *out) {
  Json *settings;
  char exe[PATH_MAX_BYTES];
  HookError e = read_settings(&settings);
  if (e != HOOKS_OK) return result(e);
  if (!current_exe(exe, sizeof exe)) e = HOOKS_IO;
  else *out = hook_config_presence(settings, exe);
  json_free(settings);
  return result(e);
}

HookResult hooks_install(void) {
  HookResult r = check_version();
  if (r.error != HOOKS_OK) return r;
  Json *settings;
  char exe[PATH_MAX_BYTES];
  HookError e = read_settings(&settings);
  if (e != HOOKS_OK) return result(e);
  if (!current_exe(exe, sizeof exe)) e = HOOKS_IO;
  else if (!hook_config_add(settings, exe)) e = HOOKS_BAD_SETTINGS;
  else e = write_settings(settings);
  json_free(settings);
  if (e == HOOKS_OK) log_info("hooks", "added, running %s", exe);
  return result(e);
}

HookResult hooks_remove(void) {
  Json *settings;
  HookError e = read_settings(&settings);
  if (e != HOOKS_OK) return result(e);
  if (hook_config_remove(settings)) {
    e = write_settings(settings);
    if (e == HOOKS_OK) log_info("hooks", "removed");
  }
  json_free(settings);
  return result(e);
}
