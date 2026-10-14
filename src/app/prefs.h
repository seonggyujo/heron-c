// Small settings kept across launches in %APPDATA%\heron-c\settings.ini.
// A convenience only: any failure falls back to the default.
#pragma once

#include <stdbool.h>
#include <stddef.h>

// Keys in use.
#define PREF_SHELL "shell"
#define PREF_LANG "lang"
#define PREF_THEME "theme"
#define PREF_FONT_SIZE "font_size"
#define PREF_MOTION "motion"
#define PREF_CLOSE_ON_EXIT "close_on_exit"
#define PREF_RESTORE_WORKSPACE "restore_workspace"
#define PREF_SIDEBAR_COLLAPSED "sidebar_collapsed"
#define PREF_PLUGIN_HINT_HIDDEN "plugin_hint_hidden"
#define PREF_HOOKS_OFFER_HIDDEN "hooks_offer_hidden"
#define PREF_FOLDED_GROUPS "folded_groups"
#define PREF_AGENT_HOOKS "agent_hooks"

// Copies the value of key into out. Returns false when it is not set.
bool prefs_get(const char *key, char *out, size_t cap);
void prefs_set(const char *key, const char *value);
void prefs_delete(const char *key);

bool prefs_get_bool(const char *key, bool fallback);
void prefs_set_bool(const char *key, bool value);
int prefs_get_int(const char *key, int fallback);
void prefs_set_int(const char *key, int value);

// Settings > Reset: forgets every setting except the Claude Code hooks
// choice (the hooks stay in Claude Code's settings).
void prefs_reset(void);
