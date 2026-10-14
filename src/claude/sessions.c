// Recent sessions, whether one can be resumed, and deleting one.
#include "claude/sessions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
// After windows.h, which defines what shellapi.h uses.
#include <shellapi.h>

#include "base/file.h"
#include "base/log.h"
#include "claude/claude_dir.h"
#include "claude/session_files.h"
#include "claude/session_summary.h"

// Bytes read from the end of a session file.
#define TAIL_BYTES (256 * 1024)

static int newest_first(const void *a, const void *b) {
  uint64_t x = ((const SessionFile *)a)->modified, y = ((const SessionFile *)b)->modified;
  return x < y ? 1 : x > y ? -1 : 0;
}

// A character cut at the start of the tail only spoils the first line,
// which session_summarize skips.
static bool read(const SessionFile *f, RecentSession *out) {
  uint64_t size = 0;
  file_info(f->path, &size, NULL);
  char *tail = file_read_range(f->path, size > TAIL_BYTES ? size - TAIL_BYTES : 0, 0, NULL);
  SessionSummary summary;
  bool ok = tail && session_summarize(tail, &summary) && file_is_dir(summary.cwd);
  free(tail);
  if (!ok) return false;
  snprintf(out->id, sizeof out->id, "%s", f->id);
  snprintf(out->cwd, sizeof out->cwd, "%s", summary.cwd);
  snprintf(out->title, sizeof out->title, "%s", summary.title);
  out->modified = f->modified;
  return true;
}

int sessions_recent(RecentSession *out, int limit) {
  char config[SESSION_PATH_MAX];
  if (!claude_config_dir(config, sizeof config)) return 0;
  RunningSessions running = sessions_running(config);
  SessionFile *files;
  int count = session_files(config, &files), n = 0;
  qsort(files, (size_t)count, sizeof *files, newest_first);
  for (int i = 0; i < count && n < limit; i++) {
    if (!session_is_id(files[i].id) || sessions_is_running(&running, files[i].id)) continue;
    if (read(&files[i], &out[n])) n++;
  }
  free(files);
  sessions_running_free(&running);
  return n;
}

// The file of session id, or false.
static bool find(const char *config, const char *id, char *path, size_t cap) {
  SessionFile *files;
  int count = session_files(config, &files);
  bool found = false;
  for (int i = 0; i < count && !found; i++) {
    found = strcmp(files[i].id, id) == 0;
    if (found) snprintf(path, cap, "%s", files[i].path);
  }
  free(files);
  return found;
}

bool sessions_resumable(const char *id) {
  char config[SESSION_PATH_MAX], path[SESSION_PATH_MAX];
  if (!session_is_id(id) || !claude_config_dir(config, sizeof config)) return false;
  RunningSessions running = sessions_running(config);
  bool ok = !sessions_is_running(&running, id) && find(config, id, path, sizeof path);
  sessions_running_free(&running);
  return ok;
}

// Moves paths (a double-NUL list) to the Recycle Bin, with undo, no dialog.
static bool recycle(const wchar_t *paths) {
  SHFILEOPSTRUCTW op = {0};
  op.wFunc = FO_DELETE;
  op.pFrom = paths;
  op.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO | FOF_NOERRORUI;
  int code = SHFileOperationW(&op);
  if (code) log_warn("sessions", "SHFileOperationW failed with 0x%x", code);
  return code == 0 && !op.fAnyOperationsAborted;
}

SessionDelete sessions_delete(const char *id) {
  char config[SESSION_PATH_MAX], path[SESSION_PATH_MAX], folder[SESSION_PATH_MAX];
  if (!session_is_id(id) || !claude_config_dir(config, sizeof config)) return SESSION_NOT_FOUND;
  RunningSessions running = sessions_running(config);
  bool busy = sessions_is_running(&running, id);
  sessions_running_free(&running);
  if (busy) return SESSION_RUNNING;
  if (!find(config, id, path, sizeof path)) return SESSION_NOT_FOUND;
  snprintf(folder, sizeof folder, "%.*s", (int)(strlen(path) - 6), path);  // without .jsonl
  wchar_t list[SESSION_PATH_MAX * 2 + 2] = {0};
  int n = MultiByteToWideChar(CP_UTF8, 0, path, -1, list, SESSION_PATH_MAX);
  if (n > 0 && file_is_dir(folder)) MultiByteToWideChar(CP_UTF8, 0, folder, -1, list + n, SESSION_PATH_MAX);
  if (n <= 0 || !recycle(list)) return SESSION_IO;
  log_info("sessions", "moved %s to the Recycle Bin", id);
  return SESSION_DELETED;
}
