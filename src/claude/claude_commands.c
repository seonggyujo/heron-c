// Claude Code command lines.
#include "claude/claude_commands.h"

#include <stdio.h>

#include "claude/session_summary.h"

const char *claude_start_command(bool worktree) { return worktree ? "claude --worktree" : "claude"; }

bool claude_resume_command(const char *id, char *out, size_t cap) {
  if (!session_is_id(id)) return false;
  snprintf(out, cap, "claude --resume %s", id);
  return true;
}
