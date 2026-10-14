// heron-c.exe run by Claude Code as a hook (`heron-c.exe --agent-hook`,
// Heron's claude_hooks/entry.rs). It writes what the hook event tells
// (hook_records.h) into the pane's files, the ones agent/ watches, and
// exits. It prints nothing, because the output of some hooks is added to
// the conversation. Outside a heron-c (or Heron) pane it exits before
// reading anything.
#pragma once

int hook_entry_run(void);
