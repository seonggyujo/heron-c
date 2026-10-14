// "Recent sessions" in the agent sidebar (Heron's ui/recent-sessions.ts,
// recent-group-head.ts, recent-session-row.ts and their CSS): Claude Code
// sessions of this PC grouped by folder, oldest at the top and newest at
// the bottom, scrolled to the newest unless the user scrolled up. A
// folder's head (arrow, name, count, how long ago) folds it; folded
// folders are remembered. A row shows the title and how long ago, or
// "Resume ↵" under the mouse; a click resumes it in a new pane, a right
// click asks in the row whether to delete it. The list scrolls with the
// wheel and has no scrollbar: a faded edge counts the sessions out of
// sight. Only ui/sidebar*.c include this.
#pragma once

#include "ui/app.h"

typedef struct {
  bool group;
  int index;  // in AgentsState.groups, or in AgentsState.shown
  Box box;    // scrolled, in window pixels
} RecentItem;

typedef struct {
  Box head, view;  // the title line, and where the items show
  int count;
  RecentItem items[RECENT_SESSIONS_MAX * 2];
  int content_h, scroll;
  int above, below;  // session rows out of sight
} RecentList;

// section: the sidebar's recent sessions box.
void recent_list_layout(App *a, Box section, RecentList *l);
SidebarHit recent_list_hit(App *a, const RecentList *l, int x, int y);
// The delete question's buttons in a session row.
void recent_ask_buttons(App *a, Box row, Box *yes, Box *no);
bool recent_folded(const App *a, const char *cwd);
void recent_toggle_fold(App *a, int group);

// sidebar_recent_paint.c
void recent_list_paint(App *a, const RecentList *l, Box section);
