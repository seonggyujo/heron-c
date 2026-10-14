// One round of watching a pane's agent files.
#include "agent/pane_watch.h"

#include <stdlib.h>
#include <string.h>

#include "base/file.h"
#include "base/log.h"

// The file's JSON when it changed since the last round.
static Json *read_changed(PaneWatch *w, const char *dir, const char *key, AgentFileKind kind) {
  char path[AGENT_PATH_MAX];
  agent_file_path(path, dir, key, kind);
  uint64_t modified;
  if (!file_info(path, NULL, &modified) || (w->seen[kind] && w->modified[kind] == modified)) return NULL;
  size_t len;
  char *text = file_read(path, &len);
  Json *data = text ? json_parse(text, len) : NULL;
  free(text);
  w->seen[kind] = data != NULL;
  w->modified[kind] = modified;
  if (!data) log_trace("agent", "pane %s file %d unreadable, next round", key, (int)kind);
  return data;
}

// The hooks name the session's transcript; a new one (a new session, or
// /clear) starts the check over.
static void follow(PaneWatch *w, const Json *state) {
  const char *path = json_string(json_get(state, "transcript_path"));
  if (!path || (w->following && strcmp(w->transcript.path, path) == 0)) return;
  transcript_follow(&w->transcript, path);
  w->following = true;
  model_check_restart(&w->check);
}

static void answered(void *ctx, const Answer *answer) { model_check_answered(ctx, answer->model); }

static void add_subagent(void *ctx, const char *id, const char *path) {
  size_t len;
  char *text = file_read(path, &len);
  Json *record = text ? json_parse(text, len) : NULL;
  free(text);
  if (!record) return;
  Json *item = json_new(JSON_OBJECT);
  json_set(item, "id", json_new_string(id));
  json_set_copy(item, "type", json_get(record, "type"));
  json_set_copy(item, "started", json_get(record, "started"));
  json_push(ctx, item);
  json_free(record);
}

static double started(const Json *item) {
  double t = 0;
  json_number(json_get(item, "started"), &t);
  return t;
}

static int by_start(const void *a, const void *b) {
  const Json *x = *(Json *const *)a, *y = *(Json *const *)b;
  double dx = started(x), dy = started(y);
  if (dx != dy) return dx < dy ? -1 : 1;
  return strcmp(json_string(json_get(x, "id")), json_string(json_get(y, "id")));
}

// [{id, type, started}] of the running subagents, oldest first.
static Json *subagents(const char *dir, const char *key) {
  Json *list = json_new(JSON_ARRAY);
  agent_subagent_files(dir, key, add_subagent, list);
  qsort(list->items, (size_t)list->count, sizeof(Json *), by_start);
  return list;
}

int pane_watch_poll(PaneWatch *w, const char *dir, int pane, AgentUpdate out[PANE_WATCH_UPDATES]) {
  char key[AGENT_KEY_MAX];
  agent_key(pane, key);
  int n = 0;
  // What a new watch counts as already sent: no answer judged, no subagent.
  if (!w->sent) w->sent = model_check_verdict(&(ModelCheck){0});
  if (!w->subagents) w->subagents = json_new(JSON_ARRAY);
  Json *state = read_changed(w, dir, key, AGENT_FILE_STATE);
  if (state) {
    follow(w, state);
    out[n++] = (AgentUpdate){"state", state};
  }
  Json *status = read_changed(w, dir, key, AGENT_FILE_STATUS);
  if (w->following) transcript_read_new(&w->transcript, answered, &w->check);
  Json *model = read_changed(w, dir, key, AGENT_FILE_MODEL);
  if (model) model_check_select(&w->check, json_string(json_get(model, "model")));
  json_free(model);
  if (status) {
    const char *selected = json_string(json_get(status, "model"));
    if (selected) model_check_select_from_status_line(&w->check, selected);
    out[n++] = (AgentUpdate){"status", status};
  }
  Json *verdict = model_check_verdict(&w->check);
  if (!json_equal(verdict, w->sent)) {
    json_free(w->sent);
    w->sent = verdict;
    out[n++] = (AgentUpdate){"check", json_copy(verdict)};
  } else {
    json_free(verdict);
  }
  Json *list = subagents(dir, key);
  if (!json_equal(list, w->subagents)) {
    json_free(w->subagents);
    w->subagents = list;
    out[n++] = (AgentUpdate){"subagents", json_copy(list)};
  } else {
    json_free(list);
  }
  return n;
}

void pane_watch_free(PaneWatch *w) {
  json_free(w->sent);
  json_free(w->subagents);
  w->sent = w->subagents = NULL;
}
