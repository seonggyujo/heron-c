// Asking git about agent folders and keeping the answers.
#include "ui/changes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/paths.h"
#include "base/mem.h"
#include "git/git.h"
#include "ui/agent_rows.h"
#include "ui/background.h"

#define WHILE_WORKING_MS 15000

typedef struct {
  char key[512], path[1024];
  bool ok;
  GitChanges changes;
} Question;

static FolderChanges *folder(AgentsState *s, const char *key, bool make) {
  for (int i = 0; i < s->folder_count; i++) {
    if (strcmp(s->folders[i].key, key) == 0) return &s->folders[i];
  }
  if (!make) return NULL;
  if (s->folder_count == s->folder_cap) {
    s->folder_cap = s->folder_cap ? s->folder_cap * 2 : 8;
    s->folders = xrealloc(s->folders, sizeof *s->folders * (size_t)s->folder_cap);
  }
  FolderChanges *f = &s->folders[s->folder_count++];
  memset(f, 0, sizeof *f);
  snprintf(f->key, sizeof f->key, "%s", key);
  return f;
}

static void ask(App *a, const char *path);

static void run_git(void *data) {
  Question *q = data;
  q->ok = git_changes(q->path, &q->changes);
}

static void answered(App *a, void *data) {
  Question *q = data;
  FolderChanges *f = folder(&a->agents, q->key, true);
  bool differs = f->known != q->ok || (q->ok && memcmp(&f->changes, &q->changes, sizeof q->changes) != 0);
  f->known = q->ok;
  f->changes = q->changes;
  f->busy = false;
  if (differs) app_redraw(a);
  if (f->again) {
    f->again = false;
    ask(a, q->path);
  }
  free(q);
}

static void ask(App *a, const char *path) {
  char key[512];
  folder_key(path, key, sizeof key);
  FolderChanges *f = folder(&a->agents, key, true);
  if (f->busy) {
    f->again = true;
    return;
  }
  f->busy = true;
  f->asked_at = GetTickCount64();
  Question *q = xcalloc(1, sizeof *q);
  snprintf(q->key, sizeof q->key, "%s", key);
  snprintf(q->path, sizeof q->path, "%s", path);
  background_run(a, run_git, answered, q);
}

static WatchedPane *watched(AgentsState *s, int pane) {
  for (int i = 0; i < s->watch_count; i++) {
    if (s->watches[i].pane == pane) return &s->watches[i];
  }
  return NULL;
}

void changes_watch(App *a) {
  AgentRow rows[AGENT_ROWS_MAX];
  int n = agent_rows(a, rows, AGENT_ROWS_MAX);
  for (int i = 0; i < a->agents.watch_count; i++) {  // a pane whose agent left starts over
    WatchedPane *w = &a->agents.watches[i];
    if (!board_get(&a->agents.board, w->pane)) w->has_state = false;
  }
  for (int i = 0; i < n; i++) {
    WatchedPane *w = watched(&a->agents, rows[i].pane);
    if (!rows[i].path || !w || (w->has_state && w->state == rows[i].agent->state)) continue;
    w->has_state = true;
    w->state = rows[i].agent->state;
    ask(a, rows[i].path);
  }
}

void changes_tick(App *a) {
  AgentRow rows[AGENT_ROWS_MAX];
  int n = agent_rows(a, rows, AGENT_ROWS_MAX);
  ULONGLONG now = GetTickCount64();
  for (int i = 0; i < n; i++) {
    if (!rows[i].path || rows[i].agent->state != AGENT_WORKING) continue;
    char key[512];
    folder_key(rows[i].path, key, sizeof key);
    FolderChanges *f = folder(&a->agents, key, false);
    if (!f || now - f->asked_at >= WHILE_WORKING_MS) ask(a, rows[i].path);
  }
}

void changes_focus(App *a) {
  AgentRow rows[AGENT_ROWS_MAX];
  int n = agent_rows(a, rows, AGENT_ROWS_MAX);
  for (int i = 0; i < n; i++) {
    if (rows[i].path) ask(a, rows[i].path);
  }
}

const GitChanges *changes_of(App *a, const char *path) {
  char key[512];
  folder_key(path, key, sizeof key);
  FolderChanges *f = folder(&a->agents, key, false);
  return f && f->known ? &f->changes : NULL;
}
