// Opening the folder menu, asking git about its folders, and its clicks.
#include "ui/folder_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/paths.h"
#include "app/strings.h"
#include "base/mem.h"
#include "git/git.h"
#include "ui/background.h"
#include "ui/folder_menu_layout.h"
#include "ui/folder_picker.h"
#include "ui/new_agent.h"
#include "ui/popover.h"

typedef struct {
  int opened, count;
  char folders[START_FOLDERS_MAX][1024];
  bool repo[START_FOLDERS_MAX];
} RepoJob;

static void ask_git(void *data) {
  RepoJob *j = data;
  char root[1024];
  for (int i = 0; i < j->count; i++) j->repo[i] = git_root(j->folders[i], root, sizeof root);
}

static void answered(App *a, void *data) {
  RepoJob *j = data;
  FolderMenu *m = &a->sidebar.menu;
  if (j->opened == m->opened) {  // still the menu git was asked for
    memcpy(m->repo, j->repo, sizeof m->repo);
    m->repo_known = true;
    app_redraw(a);
  }
  free(j);
}

void folder_menu_open(App *a) {
  FolderMenu *m = &a->sidebar.menu;
  const char *folders[START_FOLDERS_MAX];
  m->count = start_folders(new_agent_path(a), a->agents.sessions, a->agents.session_count, folders);
  for (int i = 0; i < m->count; i++) snprintf(m->folders[i], sizeof m->folders[i], "%s", folders[i]);
  m->repo_known = false;
  m->opened++;
  RepoJob *j = xcalloc(1, sizeof *j);
  j->opened = m->opened;
  j->count = m->count;
  memcpy(j->folders, m->folders, sizeof j->folders);
  background_run(a, ask_git, answered, j);
}

Box folder_menu_panel(App *a) {
  FolderMenuLayout l;
  folder_menu_layout(a, &l);
  return l.panel;
}

bool folder_menu_mouse_move(App *a, int x, int y) {
  FolderMenuLayout l;
  folder_menu_layout(a, &l);
  int item = folder_menu_item_at(&l, x, y);
  if (item != a->popover_hover) {
    a->popover_hover = item;
    app_redraw(a);
  }
  return box_contains(l.panel, x, y);
}

static void pick_folder(App *a) {
  char *dir = folder_picker_run(a->hwnd);
  if (dir) new_agent_start(a, dir, false);
  free(dir);
}

bool folder_menu_mouse_down(App *a, int x, int y) {
  FolderMenuLayout l;
  folder_menu_layout(a, &l);
  int item = folder_menu_item_at(&l, x, y);
  if (item < 0) return box_contains(l.panel, x, y);
  popover_close(a);
  const FolderMenu *m = &a->sidebar.menu;
  if (item == MENU_PICK) pick_folder(a);
  else if (item >= MENU_WORKTREE) new_agent_start(a, m->folders[item - MENU_WORKTREE], true);
  else new_agent_start(a, m->folders[item], false);
  return true;
}

const char *folder_menu_tip_at(App *a, int x, int y, Box *box, char *buf, size_t cap) {
  FolderMenuLayout l;
  folder_menu_layout(a, &l);
  int item = folder_menu_item_at(&l, x, y);
  if (item < 0 || item == MENU_PICK) return NULL;
  const FolderMenu *m = &a->sidebar.menu;
  if (item < MENU_WORKTREE) {
    *box = l.rows[item];
    format_new_agent_in(buf, cap, m->folders[item]);
    return buf;
  }
  char name[256];
  folder_name(m->folders[item - MENU_WORKTREE], name, sizeof name);
  *box = l.worktrees[item - MENU_WORKTREE];
  format_worktree_hint(buf, cap, name[0] ? name : m->folders[item - MENU_WORKTREE]);
  return buf;
}
