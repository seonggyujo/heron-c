// The heron-limits plugin's status file for one pane (Heron's
// agent/usage.ts): the account's usage limits (5-hour and weekly) and how
// much of its context window the agent uses. Pure.
#pragma once

#include <stdbool.h>

#include "base/json.h"

typedef struct {
  bool known;
  double used;  // percent
  bool has_reset;
  double resets_at;  // epoch seconds
} Limit;

typedef struct {
  Limit five_hour, seven_day;
  double at;  // when the plugin wrote them (ms), to keep the newest
} Limits;

// The limits in a status file; false when it has none (a new session's
// status line has none before its first answer, and the last known ones
// must stay on screen meanwhile).
bool usage_limits_of(const Json *data, Limits *out);
// Percent of the context window in use; false before the first answer.
bool usage_context_of(const Json *data, double *out);

// How full a meter looks: green, yellow from 70%, red from 90%.
typedef enum { FILL_OK, FILL_WARN, FILL_BAD } Fill;
Fill usage_fill(double percent);
