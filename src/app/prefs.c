// Settings in an ini file under %APPDATA%.
#include "app/prefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "base/log.h"
#include "base/utf.h"

static bool settings_path(wchar_t *out, bool create_folder) {
  DWORD n = GetEnvironmentVariableW(L"APPDATA", out, MAX_PATH);
  if (n == 0 || n + 30 >= MAX_PATH) return false;
  wcscat(out, L"\\heron-c");
  if (create_folder) CreateDirectoryW(out, NULL);
  wcscat(out, L"\\settings.ini");
  return true;
}

bool prefs_get(const char *key, char *out, size_t cap) {
  wchar_t path[MAX_PATH], value[2048];
  if (!settings_path(path, false)) return false;
  wchar_t *wkey = utf8_to_wide(key, -1);
  DWORD n = GetPrivateProfileStringW(L"heron", wkey, L"", value, 2048, path);
  free(wkey);
  if (n == 0) return false;
  WideCharToMultiByte(CP_UTF8, 0, value, -1, out, (int)cap, NULL, NULL);
  out[cap - 1] = 0;
  return true;
}

static void write_value(const char *key, const wchar_t *value) {
  wchar_t path[MAX_PATH];
  if (!settings_path(path, true)) return;
  wchar_t *wkey = utf8_to_wide(key, -1);
  if (!WritePrivateProfileStringW(L"heron", wkey, value, path)) {
    log_warn("prefs", "could not save %s: error %lu", key, GetLastError());
  }
  free(wkey);
}

void prefs_set(const char *key, const char *value) {
  wchar_t *wvalue = utf8_to_wide(value, -1);
  write_value(key, wvalue);
  free(wvalue);
  log_debug("prefs", "%s = %s", key, value);
}

void prefs_delete(const char *key) { write_value(key, NULL); }

bool prefs_get_bool(const char *key, bool fallback) {
  char v[8];
  if (!prefs_get(key, v, sizeof v)) return fallback;
  return strcmp(v, "true") == 0 ? true : strcmp(v, "false") == 0 ? false : fallback;
}

void prefs_set_bool(const char *key, bool value) { prefs_set(key, value ? "true" : "false"); }

int prefs_get_int(const char *key, int fallback) {
  char v[16];
  return prefs_get(key, v, sizeof v) ? atoi(v) : fallback;
}

void prefs_set_int(const char *key, int value) {
  char v[16];
  snprintf(v, sizeof v, "%d", value);
  prefs_set(key, v);
}

void prefs_reset(void) {
  static const char *const KEYS[] = {PREF_SHELL,   PREF_LANG,          PREF_THEME,
                                     PREF_FONT_SIZE, PREF_MOTION,      PREF_BUDDIES, PREF_CLOSE_ON_EXIT,
                                     PREF_RESTORE_WORKSPACE, PREF_SIDEBAR_COLLAPSED,
                                     PREF_PLUGIN_HINT_HIDDEN, PREF_HOOKS_OFFER_HIDDEN,
                                     PREF_FOLDED_GROUPS};
  for (size_t i = 0; i < sizeof KEYS / sizeof KEYS[0]; i++) prefs_delete(KEYS[i]);
  log_info("prefs", "settings reset to defaults");
}
