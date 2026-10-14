// The recent Claude Code sessions in the sidebar, wired like Heron's
// wire-sessions.ts: the list is read on a background thread at start, when
// the window gets focus and when a session leaves its pane; sessions
// running in panes are left out. Opening one types `claude --resume <id>`
// at the first prompt of a new pane in the folder it last ran in; deleting
// moves it to the Recycle Bin. The listed sessions and their folder groups
// are kept in AgentsState (shown, groups).
#pragma once

#include "ui/app.h"

void recent_load(App *a);
// After the board changed: notes the sessions running in panes, and reads
// the list again when one left its pane (it was just written).
void recent_running(App *a);
void recent_open(App *a, const RecentSession *s);
// done gets SESSION_DELETED or why not, on the UI thread.
typedef void (*RecentDeleted)(App *a, SessionDelete result);
void recent_delete(App *a, const char *id, RecentDeleted done);
