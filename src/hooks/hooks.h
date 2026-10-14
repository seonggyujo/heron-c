// heron-c's own Claude Code hooks (Heron's claude_hooks/mod.rs), so agents
// in the panes report what they do and which model they use, without any
// plugin. With the user's consent (the sidebar or Settings) entries go into
// Claude Code's settings.json that run `heron-c.exe --agent-hook` on a few
// events (hook_entry.c writes the pane's files). Removing them leaves the
// user's own hooks as they were. Install and presence run processes and
// read files: call them off the UI thread.
#pragma once

#include "hooks/hook_config.h"

typedef enum {
  HOOKS_OK,
  HOOKS_NO_CLAUDE,     // `claude --version` did not run
  HOOKS_CLAUDE_OLD,    // older than 2.1.139, which ignores `args`
  HOOKS_BAD_SETTINGS,  // settings.json is not a JSON object: left alone
  HOOKS_IO,
} HookError;

typedef struct {
  HookError error;
  char version[32];  // HOOKS_CLAUDE_OLD: the version found
} HookResult;

HookResult hooks_presence(HookPresence *out);
// Adds the hooks for this heron-c.exe, replacing older ones.
HookResult hooks_install(void);
HookResult hooks_remove(void);
