// The folder menu of "New agent" (Heron's ui/agent-folder-menu.ts), opened
// by its ▾ under the button: a heading, one row per folder (the selected
// pane's, then the recent sessions'; name over path), "worktree" beside a
// folder in a git repository once git answered, then "Choose a folder…".
// A click starts Claude Code there (ui/new_agent.h). Opening and closing
// is ui/popover.c.
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "ui/app.h"

// Takes the folders and asks git which are repositories.
void folder_menu_open(App *a);
void folder_menu_paint(App *a);
Box folder_menu_panel(App *a);
// True when (x, y) is inside the menu.
bool folder_menu_mouse_move(App *a, int x, int y);
bool folder_menu_mouse_down(App *a, int x, int y);
const char *folder_menu_tip_at(App *a, int x, int y, Box *box, char *buf, size_t cap);
