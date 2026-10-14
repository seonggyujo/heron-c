// Counting changes (Heron's git/numstat.rs), pure: the totals of
// `git diff --numstat` output, and the lines of a new file's text.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint64_t added;  // lines, new files included
  uint64_t removed;
  uint64_t files;  // changed, added or removed
} GitChanges;

// "<added>\t<removed>\t<path>" per line; a binary file shows "-" for both.
GitChanges numstat_parse(const char *text);
// An untracked file with lines lines (0 when not counted).
void numstat_add_new_file(GitChanges *c, uint64_t lines);
// Lines of a text file's bytes; false for binary data (a NUL byte).
bool numstat_text_lines(const char *bytes, size_t len, uint64_t *out);
