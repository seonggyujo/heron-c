// Judging answers against the selected model.
#include "agent/model_check.h"

#include <stdio.h>

static void copy(char *out, const char *text) { snprintf(out, ANSWER_TEXT_MAX, "%s", text ? text : ""); }

static const char *selected(const ModelCheck *c) {
  if (c->hooks[0]) return c->hooks;
  return c->status_line[0] ? c->status_line : NULL;
}

void model_check_select(ModelCheck *c, const char *model) { copy(c->hooks, model); }

void model_check_select_from_status_line(ModelCheck *c, const char *model) { copy(c->status_line, model); }

void model_check_restart(ModelCheck *c) {
  c->has_last = false;
  c->status_line[0] = 0;
}

void model_check_answered(ModelCheck *c, const char *model) {
  c->has_last = true;
  copy(c->last_actual, model);
  copy(c->last_selected, selected(c));
}

static Json *text_or_null(const char *text) { return text ? json_new_string(text) : json_new(JSON_NULL); }

Json *model_check_verdict(const ModelCheck *c) {
  const char *now = selected(c), *state = "pending";
  if (c->has_last && now && same_model(c->last_actual, now)) {
    state = "ok";
  } else if (c->has_last && now && c->last_selected[0] && same_model(c->last_selected, now)) {
    // Judged against what is still selected: a real mismatch. After a
    // switch, wait for the next answer instead.
    state = "mismatch";
  }
  Json *v = json_new(JSON_OBJECT);
  json_set(v, "state", json_new_string(state));
  json_set(v, "selected", text_or_null(now));
  json_set(v, "actual", text_or_null(c->has_last ? c->last_actual : NULL));
  return v;
}
