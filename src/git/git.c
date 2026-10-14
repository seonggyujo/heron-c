// Running git for a folder's changes.
#include "git/git.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/file.h"
#include "base/process.h"

// git's empty tree: what a repository without commits is compared with.
#define EMPTY_TREE "4b825dc642cb6eb9a060e54bf8d69288fbee4904"
// New files bigger than this count without their lines.
#define MAX_COUNTED_BYTES (2 * 1024 * 1024)
// Only this many new files have their lines counted.
#define MAX_COUNTED_FILES 200
#define PATH_BYTES 1024
#define TIMEOUT_MS 20000

bool git_root(const char *dir, char *out, size_t cap) {
  char at[PATH_BYTES], probe[PATH_BYTES + 8];
  snprintf(at, sizeof at, "%s", dir);
  for (;;) {
    size_t n = strlen(at);
    while (n > 3 && (at[n - 1] == '\\' || at[n - 1] == '/')) at[--n] = 0;
    file_join(probe, sizeof probe, at, ".git");
    if (file_exists(probe)) {
      snprintf(out, cap, "%s", at);
      return true;
    }
    char *slash = strrchr(at, '\\'), *other = strrchr(at, '/');
    if (other > slash) slash = other;
    if (!slash || n <= 3) return false;  // past the drive root
    slash[slash - at == 2 ? 1 : 0] = 0;  // "C:\x" goes to "C:\"
  }
}

// git's output; false when it cannot run or fails.
static bool git(const char *root, const char *args, Buf *out) {
  char line[PATH_BYTES * 2];
  size_t n = strlen(root);
  // A backslash before the closing quote would escape it: double it.
  snprintf(line, sizeof line, "git -C \"%s%s\" %s", root, n && root[n - 1] == '\\' ? "\\" : "", args);
  EnvVar env = {L"GIT_OPTIONAL_LOCKS", L"0"};
  ProcessRun run = {line, NULL, &env, 1, TIMEOUT_MS};
  bool ok = process_run(&run, out) == 0;
  buf_append(out, "", 1);
  return ok;
}

static uint64_t new_file_lines(const char *root, const char *name) {
  char path[PATH_BYTES * 2];
  uint64_t size, lines = 0;
  file_join(path, sizeof path, root, name);
  if (!file_info(path, &size, NULL) || size > MAX_COUNTED_BYTES) return 0;
  size_t len;
  char *bytes = file_read(path, &len);
  if (bytes && !numstat_text_lines(bytes, len, &lines)) lines = 0;
  free(bytes);
  return lines;
}

bool git_changes(const char *dir, GitChanges *out) {
  char root[PATH_BYTES];
  if (!file_is_dir(dir) || !git_root(dir, root, sizeof root)) return false;
  Buf tracked = {0}, untracked = {0};
  bool ok = git(root, "diff --numstat HEAD --", &tracked);
  if (!ok) {
    tracked.len = 0;
    ok = git(root, "diff --numstat " EMPTY_TREE " --", &tracked);
  }
  ok = ok && git(root, "ls-files --others --exclude-standard -z", &untracked);
  if (ok) {
    *out = numstat_parse(tracked.data);
    int i = 0;
    for (const char *name = untracked.data; name < untracked.data + untracked.len - 1; name += strlen(name) + 1, i++) {
      if (*name) numstat_add_new_file(out, i < MAX_COUNTED_FILES ? new_file_lines(root, name) : 0);
    }
  }
  buf_free(&tracked);
  buf_free(&untracked);
  return ok;
}
