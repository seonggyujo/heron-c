// Reading the hook input and writing the pane's files.
#include "hooks/hook_entry.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "agent/agent_files.h"
#include "base/clock.h"
#include "base/file.h"
#include "hooks/hook_records.h"

// A permission notice follows its request within moments.
#define RECENT_MS 30000

// The variables every pane's shell carries.
static bool pane(char dir[AGENT_PATH_MAX], char key[AGENT_KEY_MAX]) {
  wchar_t w[AGENT_PATH_MAX];
  DWORD n = GetEnvironmentVariableW(L"HERON_PANE", w, AGENT_KEY_MAX);
  if (n == 0 || n >= AGENT_KEY_MAX) return false;
  WideCharToMultiByte(CP_UTF8, 0, w, -1, key, AGENT_KEY_MAX, NULL, NULL);
  n = GetEnvironmentVariableW(L"HERON_AGENT_DIR", w, AGENT_PATH_MAX / 3);
  if (n == 0 || n >= AGENT_PATH_MAX / 3) return false;
  WideCharToMultiByte(CP_UTF8, 0, w, -1, dir, AGENT_PATH_MAX, NULL, NULL);
  return agent_is_key(key);
}

static char *read_stdin(size_t *len) {
  HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
  if (!in || in == INVALID_HANDLE_VALUE) return NULL;
  Buf b = {0};
  char chunk[8192];
  DWORD got;
  while (ReadFile(in, chunk, sizeof chunk, &got, NULL) && got) buf_append(&b, chunk, got);
  *len = b.len;
  return b.data;
}

// The pane's state record when it was written within RECENT_MS, so a
// record of an earlier prompt is not taken for this one.
static Json *recent_record(const char *path) {
  uint64_t modified;
  if (!file_info(path, NULL, &modified) || clock_now_ms() - modified > RECENT_MS) return NULL;
  size_t len;
  char *text = file_read(path, &len);
  Json *v = text ? json_parse(text, len) : NULL;
  free(text);
  return v;
}

// A failed write only loses this one update.
static void write_record(const char *path, const Json *record) {
  Buf text = {0};
  json_write(record, &text, false);
  file_write_atomic(path, text.data ? text.data : "", text.len);
  buf_free(&text);
}

static void apply_subagent(const Json *input, const char *dir, const char *key) {
  SubagentChange c = hook_subagent_change(input, clock_now_ms());
  char path[AGENT_PATH_MAX];
  if ((c.kind == SUBAGENT_START || c.kind == SUBAGENT_STOP) && agent_is_subagent_id(c.id)) {
    agent_subagent_path(path, dir, key, c.id);
    if (c.kind == SUBAGENT_START) write_record(path, c.record);
    else file_remove(path);
  } else if (c.kind == SUBAGENT_CLEAR) {
    agent_remove_subagents(dir, key);
  }
  json_free(c.record);
}

int hook_entry_run(void) {
  char dir[AGENT_PATH_MAX], key[AGENT_KEY_MAX], path[AGENT_PATH_MAX];
  if (!pane(dir, key)) return 0;
  size_t len = 0;
  char *raw = read_stdin(&len);
  if (!raw) return 0;
  // A byte order mark, as some writers put first, is not JSON.
  size_t skip = len >= 3 && memcmp(raw, "\xef\xbb\xbf", 3) == 0 ? 3 : 0;
  Json *input = json_parse(raw + skip, len - skip);
  free(raw);
  if (!input) return 0;
  agent_file_path(path, dir, key, AGENT_FILE_STATE);
  Json *previous = recent_record(path);
  Json *state = hook_state_record(input, previous);
  if (state) write_record(path, state);
  Json *model = hook_model_record(input);
  agent_file_path(path, dir, key, AGENT_FILE_MODEL);
  if (model) write_record(path, model);
  apply_subagent(input, dir, key);
  json_free(model);
  json_free(state);
  json_free(previous);
  json_free(input);
  return 0;
}
