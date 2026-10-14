// Grouping sessions by folder, and the start folders.
#include "agent/session_groups.h"

#include <stdlib.h>
#include <string.h>

#include "app/paths.h"
#include "base/mem.h"

static int oldest_first(const void *a, const void *b) {
  uint64_t x = (*(const RecentSession *const *)a)->modified, y = (*(const RecentSession *const *)b)->modified;
  return x < y ? -1 : x > y ? 1 : 0;
}

static bool same_folder(const char *a, const char *b) {
  char x[1024], y[1024];
  folder_key(a, x, sizeof x);
  folder_key(b, y, sizeof y);
  return strcmp(x, y) == 0;
}

static int by_newest_session(const void *a, const void *b) {
  uint64_t x = ((const SessionGroup *)a)->modified, y = ((const SessionGroup *)b)->modified;
  return x < y ? -1 : x > y ? 1 : 0;
}

SessionGroups session_groups(const RecentSession *sessions, int count) {
  const RecentSession **sorted = xmalloc(sizeof *sorted * (size_t)(count + 1));
  for (int i = 0; i < count; i++) sorted[i] = &sessions[i];
  qsort(sorted, (size_t)count, sizeof *sorted, oldest_first);
  SessionGroups g = {xcalloc((size_t)count + 1, sizeof(SessionGroup)), 0, xmalloc(sizeof *g.order * (size_t)(count + 1))};
  for (int i = 0; i < count; i++) {  // the newest session of a folder names it
    int k = 0;
    while (k < g.count && !same_folder(g.groups[k].cwd, sorted[i]->cwd)) k++;
    if (k == g.count) g.count++;
    g.groups[k].cwd = sorted[i]->cwd;
    g.groups[k].modified = sorted[i]->modified;
    g.groups[k].count++;
  }
  qsort(g.groups, (size_t)g.count, sizeof *g.groups, by_newest_session);
  for (int k = 0, at = 0; k < g.count; k++) {
    g.groups[k].first = at;
    g.groups[k].count = 0;
    for (int i = 0; i < count; i++) {
      if (same_folder(sorted[i]->cwd, g.groups[k].cwd)) g.order[at + g.groups[k].count++] = sorted[i];
    }
    at += g.groups[k].count;
  }
  free(sorted);
  return g;
}

void session_groups_free(SessionGroups *g) {
  free(g->groups);
  free(g->order);
  g->groups = NULL;
  g->order = NULL;
  g->count = 0;
}

int start_folders(const char *selected, const RecentSession *sessions, int count,
                  const char *out[START_FOLDERS_MAX]) {
  const RecentSession **newest = xmalloc(sizeof *newest * (size_t)(count + 1));
  for (int i = 0; i < count; i++) newest[i] = &sessions[i];
  qsort(newest, (size_t)count, sizeof *newest, oldest_first);
  int n = 0;
  for (int i = -1; i < count && n < START_FOLDERS_MAX; i++) {
    const char *folder = i < 0 ? selected : newest[count - 1 - i]->cwd;
    if (!folder || !folder[0] || in_worktree(folder)) continue;
    bool seen = false;
    for (int k = 0; k < n && !seen; k++) seen = same_folder(out[k], folder);
    if (!seen) out[n++] = folder;
  }
  free(newest);
  return n;
}
