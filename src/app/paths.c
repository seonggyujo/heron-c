// Folder names and worktree labels.
#include "app/paths.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static bool is_slash(char c) { return c == '\\' || c == '/'; }

void folder_name(const char *path, char *out, size_t cap) {
  size_t end = strlen(path);
  while (end && is_slash(path[end - 1])) end--;
  size_t start = end;
  while (start && !is_slash(path[start - 1])) start--;
  snprintf(out, cap, "%.*s", (int)(end - start), path + start);
}

// The repository part and the worktree name of a worktree path.
static bool worktree(const char *path, size_t *repo_len, const char **name, size_t *name_len) {
  static const char MARK[] = ".claude";
  for (const char *p = path; (p = strstr(p, MARK)); p++) {
    if (p == path || !is_slash(p[-1]) || !is_slash(p[7]) || strncmp(p + 8, "worktrees", 9) != 0 || !is_slash(p[17])) {
      continue;
    }
    const char *n = p + 18;
    size_t len = 0;
    while (n[len] && !is_slash(n[len])) len++;
    if (len == 0) continue;
    *repo_len = (size_t)(p - 1 - path);
    *name = n;
    *name_len = len;
    return true;
  }
  return false;
}

bool in_worktree(const char *path) {
  size_t repo_len, name_len;
  const char *name;
  return worktree(path, &repo_len, &name, &name_len);
}

void folder_label(const char *path, char *out, size_t cap) {
  size_t repo_len, name_len;
  const char *name;
  if (!worktree(path, &repo_len, &name, &name_len)) {
    folder_name(path, out, cap);
    return;
  }
  char repo[1024], repo_name[256];
  snprintf(repo, sizeof repo, "%.*s", (int)repo_len, path);
  folder_name(repo, repo_name, sizeof repo_name);
  snprintf(out, cap, "%s \xc2\xb7 %.*s", repo_name[0] ? repo_name : repo, (int)name_len, name);
}

void folder_key(const char *path, char *out, size_t cap) {
  size_t end = strlen(path);
  while (end && is_slash(path[end - 1])) end--;
  size_t n = end < cap - 1 ? end : cap - 1;
  for (size_t i = 0; i < n; i++) out[i] = (char)tolower((unsigned char)path[i]);
  out[n] = 0;
}
