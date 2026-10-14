// Recent sessions grouped by the folder they last ran in (Heron's
// agent/session-groups.ts), pure. The folder whose newest session is
// oldest comes first, so the newest is at the bottom, and the sessions in
// a folder are in the same order. Folders that differ only in case or a
// trailing backslash are one. Also the folders the "New agent" menu offers
// (start-folders.ts).
#pragma once

#include "claude/sessions.h"

typedef struct {
  const char *cwd;  // as its newest session wrote it
  int first, count;  // its sessions: order[first .. first + count), oldest first
  uint64_t modified;  // last write of the newest session
} SessionGroup;

typedef struct {
  SessionGroup *groups;
  int count;
  const RecentSession **order;  // the sessions, grouped
} SessionGroups;

SessionGroups session_groups(const RecentSession *sessions, int count);
void session_groups_free(SessionGroups *g);

#define START_FOLDERS_MAX 8

// The selected pane's folder first (when given), then the recent sessions'
// folders, newest first, each once, without the worktrees Claude Code made
// (a new agent starts in the repository; its worktree comes from
// --worktree). Returns how many; out points into the inputs.
int start_folders(const char *selected, const RecentSession *sessions, int count,
                  const char *out[START_FOLDERS_MAX]);
