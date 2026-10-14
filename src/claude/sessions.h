// Claude Code's sessions on this PC, for the sidebar (Heron's sessions/):
// the newest session files under <config folder>\projects\<project>\, each
// read from its end (session_summary.h), and deleting one. Claude Code
// writes the title records again on every prompt, so the end holds the
// latest. Sessions some Claude Code runs now (each running process keeps
// sessions\<pid>.json) and sessions whose folder is gone are left out.
// These read many files: call them off the UI thread.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char id[40];  // a UUID, safe to type after `claude --resume`
  char cwd[1024];
  char title[512];
  uint64_t modified;  // last write, ms since 1970
} RecentSession;

// The `limit` most recently written sessions, newest first; returns how many.
int sessions_recent(RecentSession *out, int limit);

// Whether `claude --resume id` can open the session: Claude Code saved it
// (it writes the file at the first message) and nothing runs it now.
bool sessions_resumable(const char *id);

typedef enum { SESSION_DELETED, SESSION_NOT_FOUND, SESSION_RUNNING, SESSION_IO } SessionDelete;
// Moves a session to the Recycle Bin: its file and the folder beside it
// (subagents, saved tool output).
SessionDelete sessions_delete(const char *id);
