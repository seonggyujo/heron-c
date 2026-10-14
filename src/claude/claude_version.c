// Reading and comparing Claude Code versions.
#include "claude/claude_version.h"

static bool number(const char **p, unsigned *out) {
  if (**p < '0' || **p > '9') return false;
  unsigned n = 0;
  while (**p >= '0' && **p <= '9') n = n * 10 + (unsigned)(*(*p)++ - '0');
  *out = n;
  return true;
}

bool claude_version_parse(const char *text, ClaudeVersion *out) {
  const char *p = text;
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
  ClaudeVersion v;
  if (!number(&p, &v.major) || *p++ != '.' || !number(&p, &v.minor) || *p++ != '.' || !number(&p, &v.patch)) {
    return false;
  }
  if (*p && *p != '.' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') return false;  // not "2.1.3x"
  *out = v;
  return true;
}

int claude_version_compare(ClaudeVersion a, ClaudeVersion b) {
  if (a.major != b.major) return a.major < b.major ? -1 : 1;
  if (a.minor != b.minor) return a.minor < b.minor ? -1 : 1;
  if (a.patch != b.patch) return a.patch < b.patch ? -1 : 1;
  return 0;
}
