// The Claude Code command lines heron-c types into a pane's shell (Heron's
// app/claude-commands.ts): start a new session (in a new git worktree of
// its own branch with --worktree), or resume one. Only a session id (a
// UUID) is ever typed after --resume.
#pragma once

#include <stdbool.h>
#include <stddef.h>

const char *claude_start_command(bool worktree);
// False for anything but a session id.
bool claude_resume_command(const char *id, char *out, size_t cap);
