// Listing session files and running sessions.
#include "claude/session_files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "base/file.h"
#include "base/json.h"
#include "base/mem.h"

typedef struct {
  const char *dir;
  SessionFile *list;
  int count, cap;
} Collect;

static bool add_file(void *ctx, const char *name, bool is_dir) {
  Collect *c = ctx;
  size_t n = strlen(name);
  if (is_dir || n <= 6 || strcmp(name + n - 6, ".jsonl") != 0 || n - 6 >= sizeof c->list->id) return true;
  if (c->count == c->cap) {
    c->cap = c->cap ? c->cap * 2 : 64;
    c->list = xrealloc(c->list, sizeof(SessionFile) * (size_t)c->cap);
  }
  SessionFile *f = &c->list[c->count];
  snprintf(f->id, sizeof f->id, "%.*s", (int)(n - 6), name);
  file_join(f->path, sizeof f->path, c->dir, name);
  if (file_info(f->path, NULL, &f->modified)) c->count++;
  return true;
}

typedef struct {
  const char *projects;
  Collect *collect;
} Projects;

static bool add_project(void *ctx, const char *name, bool is_dir) {
  Projects *p = ctx;
  if (!is_dir) return true;
  char dir[SESSION_PATH_MAX];
  file_join(dir, sizeof dir, p->projects, name);
  p->collect->dir = dir;
  file_list(dir, add_file, p->collect);
  return true;
}

int session_files(const char *config, SessionFile **out) {
  char projects[SESSION_PATH_MAX];
  file_join(projects, sizeof projects, config, "projects");
  Collect c = {NULL, NULL, 0, 0};
  Projects p = {projects, &c};
  file_list(projects, add_project, &p);
  *out = c.list;
  return c.count;
}

static bool alive(DWORD pid) {
  HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (!h) return false;
  DWORD code = 0;
  bool running = GetExitCodeProcess(h, &code) && code == STILL_ACTIVE;
  CloseHandle(h);
  return running;
}

typedef struct {
  const char *dir;
  RunningSessions *r;
} Running;

static bool add_running(void *ctx, const char *name, bool is_dir) {
  Running *s = ctx;
  size_t n = strlen(name);
  if (is_dir || n <= 5 || strcmp(name + n - 5, ".json") != 0) return true;
  char path[SESSION_PATH_MAX];
  file_join(path, sizeof path, s->dir, name);
  size_t len;
  char *text = file_read(path, &len);
  Json *record = text ? json_parse(text, len) : NULL;
  free(text);
  double pid;
  const char *id = json_string(json_get(record, "sessionId"));
  if (id && strlen(id) < 40 && json_number(json_get(record, "pid"), &pid) && pid > 0 && pid < 4294967296.0 &&
      alive((DWORD)pid)) {
    s->r->ids = xrealloc(s->r->ids, sizeof *s->r->ids * (size_t)(s->r->count + 1));
    snprintf(s->r->ids[s->r->count++], 40, "%s", id);
  }
  json_free(record);
  return true;
}

RunningSessions sessions_running(const char *config) {
  RunningSessions r = {0, NULL};
  char dir[SESSION_PATH_MAX];
  file_join(dir, sizeof dir, config, "sessions");
  Running s = {dir, &r};
  file_list(dir, add_running, &s);
  return r;
}

bool sessions_is_running(const RunningSessions *r, const char *id) {
  for (int i = 0; i < r->count; i++) {
    if (strcmp(r->ids[i], id) == 0) return true;
  }
  return false;
}

void sessions_running_free(RunningSessions *r) {
  free(r->ids);
  r->ids = NULL;
  r->count = 0;
}
