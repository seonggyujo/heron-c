// heron-c's entries in the `hooks` of Claude Code's settings (Heron's
// claude_hooks/config.rs), as pure edits of the parsed file: add them
// (replacing older ones), remove them, and tell whether they are there.
// An entry is ours when it runs heron-c.exe with --agent-hook; the user's
// own hooks, and Heron's (heron.exe), are never touched.
#pragma once

#include <stdbool.h>

#include "base/json.h"

#define HOOK_FLAG "--agent-hook"
#define HOOK_REMOVE_FLAG "--remove-agent-hooks"

typedef enum {
  HOOKS_ABSENT,
  HOOKS_CURRENT,  // every event runs this heron-c.exe
  HOOKS_STALE,    // some are missing or run another heron-c.exe
} HookPresence;

HookPresence hook_config_presence(const Json *settings, const char *exe);
// Puts our entries in, running exe. False when the file's shape is not
// what Claude Code expects; settings must then not be written.
bool hook_config_add(Json *settings, const char *exe);
// Takes our entries out, and any group, event or `hooks` object that only
// they filled. Returns whether anything changed.
bool hook_config_remove(Json *settings);
