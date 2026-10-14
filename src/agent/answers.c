// Transcript lines to answers, and model comparison.
#include "agent/answers.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "base/json.h"
#include "claude/claude_version.h"

// Claude Code tells hooks about model switches since this version.
static const ClaudeVersion MODEL_SWITCH_HOOK = {2, 1, 251};

static bool too_old(const Json *entry) {
  ClaudeVersion v;
  const char *version = json_string(json_get(entry, "version"));
  return version && claude_version_parse(version, &v) && claude_version_compare(v, MODEL_SWITCH_HOOK) < 0;
}

bool answer_of(const char *line, size_t len, Answer *out) {
  // Cheap test first: most lines are not answers.
  if (!strstr(line, "\"assistant\"")) return false;
  Json *entry = json_parse(line, len);
  const Json *message = json_get(entry, "message");
  const char *type = json_string(json_get(entry, "type"));
  const char *model = json_string(json_get(message, "model"));
  bool ok = type && strcmp(type, "assistant") == 0 && !json_true(json_get(entry, "isSidechain")) && !too_old(entry) &&
            model && model[0] && strcmp(model, "<synthetic>") != 0;
  if (ok) {
    const char *id = json_string(json_get(message, "id"));
    if (!id) id = json_string(json_get(entry, "uuid"));
    snprintf(out->id, sizeof out->id, "%s", id ? id : "");
    snprintf(out->model, sizeof out->model, "%s", model);
    const char *ts = json_string(json_get(entry, "timestamp"));
    out->has_at = ts && iso_millis(ts, &out->at);
  }
  json_free(entry);
  return ok;
}

// Lowercase, without a [..] suffix or an 8-digit date at the end.
static void base_model(const char *id, char *out, size_t cap) {
  size_t n = strlen(id);
  const char *bracket = strchr(id, '[');
  if (bracket && n && id[n - 1] == ']') n = (size_t)(bracket - id);
  const char *dash = NULL;
  for (size_t i = 0; i < n; i++) {
    if (id[i] == '-') dash = id + i;
  }
  if (dash && (size_t)(id + n - dash - 1) == 8) {
    bool date = true;
    for (const char *p = dash + 1; p < id + n; p++) date &= isdigit((unsigned char)*p) != 0;
    if (date) n = (size_t)(dash - id);
  }
  size_t k = n < cap - 1 ? n : cap - 1;
  for (size_t i = 0; i < k; i++) out[i] = (char)tolower((unsigned char)id[i]);
  out[k] = 0;
}

bool same_model(const char *a, const char *b) {
  char x[ANSWER_TEXT_MAX], y[ANSWER_TEXT_MAX];
  base_model(a, x, sizeof x);
  base_model(b, y, sizeof y);
  return strcmp(x, y) == 0;
}

// Days since 1970-01-01 (Howard Hinnant's algorithm).
static int64_t days_from_civil(int64_t year, int64_t month, int64_t day) {
  year -= month <= 2;
  int64_t era = (year >= 0 ? year : year - 399) / 400;
  int64_t year_of_era = year - era * 400;
  int64_t day_of_year = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  int64_t day_of_era = year_of_era * 365 + year_of_era / 4 - year_of_era / 100 + day_of_year;
  return era * 146097 + day_of_era - 719468;
}

static bool num(const char *ts, int from, int to, int64_t *out) {
  int64_t n = 0;
  for (int i = from; i < to; i++) {
    if (!isdigit((unsigned char)ts[i])) return false;
    n = n * 10 + (ts[i] - '0');
  }
  *out = n;
  return true;
}

bool iso_millis(const char *ts, uint64_t *out) {
  size_t len = strlen(ts);
  if (len < 20 || ts[4] != '-' || ts[7] != '-' || ts[10] != 'T' || ts[13] != ':' || ts[16] != ':' || ts[len - 1] != 'Z') {
    return false;
  }
  int64_t year, month, day, hour, minute, second, millis = 0;
  if (!num(ts, 0, 4, &year) || !num(ts, 5, 7, &month) || !num(ts, 8, 10, &day) || !num(ts, 11, 13, &hour) ||
      !num(ts, 14, 16, &minute) || !num(ts, 17, 19, &second)) {
    return false;
  }
  size_t fraction = len - 1 - 19;  // between the seconds and the Z
  if (fraction) {
    if (ts[19] != '.' || fraction < 2) return false;
    int digits = (int)(fraction - 1 < 3 ? fraction - 1 : 3);
    if (!num(ts, 20, 20 + digits, &millis)) return false;
    for (int i = digits; i < 3; i++) millis *= 10;
  }
  int64_t days = days_from_civil(year, month, day);
  if (days < 0) return false;
  *out = (uint64_t)((days * 86400 + hour * 3600 + minute * 60 + second) * 1000 + millis);
  return true;
}
