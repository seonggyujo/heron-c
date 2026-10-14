// The work of the last run (Heron's wire-workspace.ts). At start (Settings
// > Reopen last work) the saved panes open again, each with its shell in
// its folder, then the user's own arrangement; a pane that ran Claude Code
// resumes its session, or starts a new one when that cannot be resumed
// (never saved because it got no message, or running elsewhere now). Then
// a pane in the folder from the command line, or one in the home folder
// when nothing else opened. While the app runs, every clock tick saves what
// is open when it changed (app/workspace.h).
#pragma once

#include "ui/app.h"

void saved_work_open(App *a);
// TIMER_CLOCK.
void saved_work_tick(App *a);
