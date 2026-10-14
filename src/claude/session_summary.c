// Reading a session file's tail.
#include "claude/session_summary.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/json.h"

#define TITLE_CHARS 80

// The value of key when it is a string with more than white space.
static const char *text(const Json *entry, const char *key) {
  const char *s = json_string(json_get(entry, key));
  if (!s) return NULL;
  for (const char *p = s; *p; p++) {
    if (!isspace((unsigned char)*p)) return s;
  }
  return NULL;
}

// The first line that is not blank, trimmed, cut to TITLE_CHARS
// characters and an ellipsis.
static void first_line(const char *s, char *out, size_t cap) {
  for (;;) {
    while (*s == ' ' || *s == '\t' || *s == '\r') s++;
    if (*s != '\n') break;
    s++;
  }
  size_t end = strcspn(s, "\n");
  while (end && isspace((unsigned char)s[end - 1])) end--;
  size_t at = 0;
  int chars = 0;
  while (at < end && chars < TITLE_CHARS) {
    at++;
    while (at < end && ((unsigned char)s[at] & 0xC0) == 0x80) at++;
    chars++;
  }
  snprintf(out, cap, "%.*s%s", (int)at, s, at < end ? "\xe2\x80\xa6" : "");
}

bool session_summarize(const char *tail, SessionSummary *out) {
  char *copy = _strdup(tail);
  const char *kinds[3] = {"custom-title", "ai-title", "last-prompt"}, *keys[3] = {"customTitle", "aiTitle", "lastPrompt"};
  char *titles[3] = {NULL, NULL, NULL}, *cwd = NULL;
  for (char *line = strtok(copy, "\n"); line; line = strtok(NULL, "\n")) {
    Json *entry = json_parse(line, strlen(line));
    if (!entry) continue;
    const char *dir = text(entry, "cwd"), *type = json_string(json_get(entry, "type"));
    if (dir) {
      free(cwd);
      cwd = _strdup(dir);
    }
    for (int k = 0; k < 3; k++) {
      const char *title = type && strcmp(type, kinds[k]) == 0 ? text(entry, keys[k]) : NULL;
      if (!title) continue;
      free(titles[k]);
      titles[k] = _strdup(title);
    }
    json_free(entry);
  }
  const char *title = titles[0] ? titles[0] : titles[1] ? titles[1] : titles[2];
  bool ok = title && cwd;
  if (ok) {
    snprintf(out->cwd, sizeof out->cwd, "%s", cwd);
    first_line(title, out->title, sizeof out->title);
  }
  for (int k = 0; k < 3; k++) free(titles[k]);
  free(cwd);
  free(copy);
  return ok;
}

bool session_is_id(const char *id) {
  if (strlen(id) != 36) return false;
  for (int i = 0; i < 36; i++) {
    bool dash = i == 8 || i == 13 || i == 18 || i == 23;
    if (dash ? id[i] != '-' : !isxdigit((unsigned char)id[i])) return false;
  }
  return true;
}
