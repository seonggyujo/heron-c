// Totals of numstat output and line counts.
#include "git/numstat.h"

#include <stdlib.h>
#include <string.h>

// A count field; "-" (binary) and anything else that is not a number is 0.
static uint64_t count(const char *from, const char *to) {
  uint64_t n = 0;
  for (const char *p = from; p < to; p++) {
    if (*p < '0' || *p > '9') return 0;
    n = n * 10 + (uint64_t)(*p - '0');
  }
  return n;
}

GitChanges numstat_parse(const char *text) {
  GitChanges c = {0, 0, 0};
  for (const char *line = text; *line;) {
    const char *end = strchr(line, '\n');
    if (!end) end = line + strlen(line);
    const char *tab1 = memchr(line, '\t', (size_t)(end - line));
    const char *tab2 = tab1 ? memchr(tab1 + 1, '\t', (size_t)(end - tab1 - 1)) : NULL;
    if (tab2) {
      c.added += count(line, tab1);
      c.removed += count(tab1 + 1, tab2);
      c.files++;
    }
    line = *end ? end + 1 : end;
  }
  return c;
}

void numstat_add_new_file(GitChanges *c, uint64_t lines) {
  c->added += lines;
  c->files++;
}

bool numstat_text_lines(const char *bytes, size_t len, uint64_t *out) {
  if (memchr(bytes, 0, len)) return false;
  uint64_t breaks = 0;
  for (size_t i = 0; i < len; i++) breaks += bytes[i] == '\n';
  *out = breaks + (len && bytes[len - 1] != '\n');
  return true;
}
