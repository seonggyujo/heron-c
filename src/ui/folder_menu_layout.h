// Where the folder menu and its rows are (Heron's agent-folder-menu.css:
// 300 px wide, 6 px under the "New agent" button, left-aligned with it).
// Only ui/folder_menu*.c include this.
#pragma once

#include "ui/app.h"

// popover_hover values: a folder row is its index, its "worktree" this
// much more, and the folder picker is MENU_PICK.
#define MENU_WORKTREE 16
#define MENU_PICK 32

typedef struct {
  Box panel, head;
  int count;
  Box rows[START_FOLDERS_MAX];      // the folder buttons
  Box worktrees[START_FOLDERS_MAX];  // w 0 while unknown or not a repository
  Box pick;
} FolderMenuLayout;

void folder_menu_layout(App *a, FolderMenuLayout *l);
// MENU_* or a row index under (x, y); -1 for none.
int folder_menu_item_at(const FolderMenuLayout *l, int x, int y);
