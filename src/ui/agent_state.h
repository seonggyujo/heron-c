// What the window knows about the coding agents, data only (App holds
// one): the agent board and each pane's file watch, the recent sessions,
// the git changes of the agents' folders, and whether heron-c's Claude Code
// hooks are on. ui/agents.c, ui/changes.c, ui/recent.c and ui/hooks_state.c
// keep it; the sidebar and the usage bar draw from it.
#pragma once

#include <stdbool.h>
#include <windows.h>

#include "agent/agent_board.h"
#include "agent/pane_watch.h"
#include "agent/session_groups.h"
#include "claude/sessions.h"
#include "git/numstat.h"

typedef struct {
  int pane;
  PaneWatch watch;
  bool has_state;
  AgentState state;  // the state git was last asked about
} WatchedPane;

typedef struct {
  char key[512];  // app/paths.h folder_key
  bool known;     // git answered, inside a repository
  GitChanges changes;
  ULONGLONG asked_at;
  bool busy, again;  // asked while busy: ask once more when the answer comes
} FolderChanges;

#define RECENT_SESSIONS_MAX 20

typedef struct {
  AgentBoard board;
  WatchedPane *watches;
  int watch_count, watch_cap;
  FolderChanges *folders;
  int folder_count, folder_cap;
  RecentSession sessions[RECENT_SESSIONS_MAX];  // newest first
  int session_count;
  bool sessions_busy, sessions_again;
  char running[16][64];  // ids of the sessions running in panes
  int running_count;
  RecentSession shown[RECENT_SESSIONS_MAX];  // the listed ones: not running in a pane
  int shown_count;
  SessionGroups groups;  // the listed ones by folder
  bool hooks_on, hooks_busy;
  char hooks_error[320];  // why turning them on or off failed
  int hover_pane;         // the pane whose sidebar row is under the mouse
} AgentsState;
