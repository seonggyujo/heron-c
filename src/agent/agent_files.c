// Agent file names and cleanup.
#include "agent/agent_files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "base/clock.h"
#include "base/file.h"
#include "base/log.h"
#include "base/utf.h"

#define STALE_MS (24ULL * 60 * 60 * 1000)

static const char *KIND_NAMES[AGENT_FILE_KINDS] = {"status", "state", "model"};

const char *agent_dir(void) {
  static char dir[AGENT_PATH_MAX];
  if (dir[0]) return dir;
  wchar_t base[MAX_PATH];
  DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", base, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return NULL;
  char *utf8 = wide_to_utf8(base, -1, NULL);
  snprintf(dir, sizeof dir, "%s\\heron-c\\agents", utf8);
  free(utf8);
  if (!file_make_dirs(dir)) log_warn("agent", "cannot create %s", dir);
  return dir;
}

void agent_key(int pane, char out[AGENT_KEY_MAX]) {
  snprintf(out, AGENT_KEY_MAX, "%lu-%d", GetCurrentProcessId(), pane);
}

static bool digits(const char *from, const char *to) {
  if (from == to) return false;
  for (const char *p = from; p < to; p++) {
    if (*p < '0' || *p > '9') return false;
  }
  return true;
}

bool agent_is_key(const char *text) {
  const char *dash = strchr(text, '-');
  return dash && digits(text, dash) && digits(dash + 1, text + strlen(text));
}

bool agent_is_subagent_id(const char *text) {
  size_t n = strlen(text);
  if (n == 0 || n > 64) return false;
  for (size_t i = 0; i < n; i++) {
    char c = text[i];
    bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
    if (!ok) return false;
  }
  return true;
}

void agent_file_path(char out[AGENT_PATH_MAX], const char *dir, const char *key, AgentFileKind kind) {
  snprintf(out, AGENT_PATH_MAX, "%s\\%s.%s.json", dir, key, KIND_NAMES[kind]);
}

void agent_subagent_path(char out[AGENT_PATH_MAX], const char *dir, const char *key, const char *id) {
  snprintf(out, AGENT_PATH_MAX, "%s\\%s.sub.%s.json", dir, key, id);
}

typedef struct {
  const char *dir, *prefix;
  SubagentVisit fn;
  void *ctx;
} SubagentScan;

static bool visit_subagent(void *ctx, const char *name, bool is_dir) {
  SubagentScan *s = ctx;
  size_t prefix = strlen(s->prefix), n = strlen(name);
  if (is_dir || n <= prefix + 5 || strncmp(name, s->prefix, prefix) != 0 || strcmp(name + n - 5, ".json") != 0) {
    return true;
  }
  char id[80];
  size_t id_len = n - prefix - 5;
  if (id_len >= sizeof id) return true;
  memcpy(id, name + prefix, id_len);
  id[id_len] = 0;
  if (!agent_is_subagent_id(id)) return true;
  char path[AGENT_PATH_MAX];
  file_join(path, sizeof path, s->dir, name);
  s->fn(s->ctx, id, path);
  return true;
}

void agent_subagent_files(const char *dir, const char *key, SubagentVisit fn, void *ctx) {
  char prefix[AGENT_KEY_MAX + 8];
  snprintf(prefix, sizeof prefix, "%s.sub.", key);
  SubagentScan s = {dir, prefix, fn, ctx};
  file_list(dir, visit_subagent, &s);
}

static void remove_one(void *ctx, const char *id, const char *path) {
  if (file_remove(path)) log_debug("agent", "removed %s", path);
}

void agent_remove_subagents(const char *dir, const char *key) { agent_subagent_files(dir, key, remove_one, NULL); }

void agent_remove_pane(const char *dir, int pane) {
  char key[AGENT_KEY_MAX], path[AGENT_PATH_MAX];
  agent_key(pane, key);
  for (int k = 0; k < AGENT_FILE_KINDS; k++) {
    agent_file_path(path, dir, key, (AgentFileKind)k);
    if (file_remove(path)) log_debug("agent", "removed %s", path);
  }
  agent_remove_subagents(dir, key);
}

static bool visit_stale(void *ctx, const char *name, bool is_dir) {
  const char *dir = ctx;
  char path[AGENT_PATH_MAX];
  uint64_t modified;
  file_join(path, sizeof path, dir, name);
  if (!is_dir && file_info(path, NULL, &modified) && clock_now_ms() - modified > STALE_MS && file_remove(path)) {
    log_debug("agent", "removed stale %s", path);
  }
  return true;
}

void agent_remove_stale(const char *dir) { file_list(dir, visit_stale, (void *)dir); }
