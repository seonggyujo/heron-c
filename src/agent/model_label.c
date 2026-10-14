// Model ids to short names. The shape read is Heron's pattern
//   ^(?:claude-)?([a-z]+)-(\d+)(?:-(\d{1,2}))?(?:-\d{8})?(\[1m\])?$  (any case)
#include "agent/model_label.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool starts(const char *s, const char *prefix) { return _strnicmp(s, prefix, strlen(prefix)) == 0; }

static size_t digit_run(const char *s) {
  size_t n = 0;
  while (isdigit((unsigned char)s[n])) n++;
  return n;
}

// The parts of a well-shaped id; false for anything else.
static bool read_id(const char *id, char *family, size_t cap, const char **major, size_t *major_len,
                    const char **minor, size_t *minor_len, bool *long_context) {
  const char *p = starts(id, "claude-") ? id + 7 : id;
  size_t f = 0;
  while (isalpha((unsigned char)p[f])) f++;
  if (f == 0 || f >= cap || p[f] != '-') return false;
  snprintf(family, cap, "%.*s", (int)f, p);
  p += f + 1;
  *major = p;
  *major_len = digit_run(p);
  if (*major_len == 0) return false;
  p += *major_len;
  *minor = NULL;
  *minor_len = 0;
  size_t run = *p == '-' ? digit_run(p + 1) : 0;
  if (run >= 1 && run <= 2) {
    *minor = p + 1;
    *minor_len = run;
    p += 1 + run;
    run = *p == '-' ? digit_run(p + 1) : 0;
  }
  if (run == 8) p += 9;
  *long_context = _stricmp(p, "[1m]") == 0;
  return *p == 0 || *long_context;
}

void model_label(const char *id, char *out, size_t cap) {
  char family[64];
  const char *major, *minor;
  size_t major_len, minor_len;
  bool long_context;
  if (read_id(id, family, sizeof family, &major, &major_len, &minor, &minor_len, &long_context)) {
    snprintf(out, cap, "%c%s %.*s%s%.*s%s", toupper((unsigned char)family[0]), family + 1, (int)major_len, major,
             minor ? "." : "", (int)minor_len, minor ? minor : "", long_context ? " 1M" : "");
    return;
  }
  const char *p = starts(id, "claude-") ? id + 7 : id;
  size_t n = strlen(p);
  bool suffix = n >= 4 && _stricmp(p + n - 4, "[1m]") == 0;
  snprintf(out, cap, "%.*s%s", (int)(suffix ? n - 4 : n), p, suffix ? " 1M" : "");
  if (out[0]) out[0] = (char)toupper((unsigned char)out[0]);
}
