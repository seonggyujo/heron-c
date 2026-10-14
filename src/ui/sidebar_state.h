// What the agent sidebar remembers between frames, data only (App holds
// one): collapsed or not and since when (its width glides), how far each
// list is scrolled, what the mouse is over or pressed, the delete question
// of a recent session, and the folders of the "New agent" menu.
// ui/sidebar*.c and ui/folder_menu*.c keep it.
#pragma once

#include <stdbool.h>
#include <windows.h>

#include "agent/session_groups.h"

// The parts of the sidebar that react to the mouse.
typedef enum {
  SIDEBAR_NONE,
  SIDEBAR_TOGGLE,
  SIDEBAR_NEW,       // "+ New agent"
  SIDEBAR_NEW_MENU,  // its ▾
  SIDEBAR_AGENT,     // index: pane id
  SIDEBAR_GROUP,     // index: folder group
  SIDEBAR_SESSION,   // index: in AgentsState.shown
  SIDEBAR_ASK,  // the delete question over a row, index: its session
  SIDEBAR_ASK_YES,
  SIDEBAR_ASK_NO,
  SIDEBAR_HOOKS_ON,
  SIDEBAR_OFFER_CLOSE,
  SIDEBAR_PLUGIN_CLOSE,
} SidebarPart;

typedef struct {
  SidebarPart part;
  int index;
} SidebarHit;

// The folders of the open "New agent" menu, copied when it opened, and
// which are in a git repository (repo_known once git answered).
typedef struct {
  int count;
  char folders[START_FOLDERS_MAX][1024];
  bool repo[START_FOLDERS_MAX], repo_known;
  int opened;  // counts openings: a git answer for an older one is dropped
} FolderMenu;

typedef struct {
  bool collapsed;
  ULONGLONG toggled_at;  // 0 when the width is not moving
  bool offer_hidden, plugin_hidden;  // notes closed for good
  char folded[2048];  // folded folders of the recent sessions: '|' and a folder key each
  int agents_scroll, recent_scroll;  // pixels scrolled down
  bool recent_newest;  // the recent list stays scrolled to the newest
  SidebarHit hover, pressed;
  char asking[40];   // the session whose delete question shows, "" for none
  bool deleting;     // its delete runs
  const char *note;  // a failure shown instead of the question
  ULONGLONG note_until;
  FolderMenu menu;
} SidebarState;
