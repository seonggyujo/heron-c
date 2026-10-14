// Reading limits and context use from a status file.
#include "agent/usage.h"

#include <math.h>

static bool finite_number(const Json *v, double *out) { return json_number(v, out) && isfinite(*out); }

static Limit limit(const Json *v) {
  Limit l = {0};
  l.known = finite_number(json_get(v, "used"), &l.used);
  l.has_reset = l.known && finite_number(json_get(v, "resets_at"), &l.resets_at);
  return l;
}

bool usage_limits_of(const Json *data, Limits *out) {
  Limits l = {0};
  if (!finite_number(json_get(data, "at"), &l.at)) return false;
  const Json *limits = json_get(data, "limits");
  l.five_hour = limit(json_get(limits, "five_hour"));
  l.seven_day = limit(json_get(limits, "seven_day"));
  if (!l.five_hour.known && !l.seven_day.known) return false;
  *out = l;
  return true;
}

bool usage_context_of(const Json *data, double *out) { return finite_number(json_get(data, "context"), out); }

Fill usage_fill(double percent) {
  if (percent >= 90) return FILL_BAD;
  return percent >= 70 ? FILL_WARN : FILL_OK;
}
