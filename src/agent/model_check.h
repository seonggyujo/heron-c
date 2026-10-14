// The model check of one pane (Heron's agent/model_check.rs): each new
// answer is judged once, against the model selected when it arrived, so
// switching models later does not turn an earlier answer into a mismatch.
// The selection comes from the hooks (SessionStart, PostModelSwitch);
// while they do not know it (a resumed session), from the heron-limits
// status line. The answers come from the transcript. Pure.
#pragma once

#include <stdbool.h>

#include "agent/answers.h"
#include "base/json.h"

typedef struct {
  char hooks[ANSWER_TEXT_MAX];        // what the hooks reported; empty: unknown
  char status_line[ANSWER_TEXT_MAX];  // what the status line of this session reported
  bool has_last;                      // an answer was judged
  char last_actual[ANSWER_TEXT_MAX];
  char last_selected[ANSWER_TEXT_MAX];  // the selection when it came; empty: unknown
} ModelCheck;

// The hooks told the session's model, or (NULL) that it is unknown.
void model_check_select(ModelCheck *c, const char *model);
void model_check_select_from_status_line(ModelCheck *c, const char *model);
// A new transcript (a new session, or /clear): no answer yet, and the
// status line has not spoken for it.
void model_check_restart(ModelCheck *c);
void model_check_answered(ModelCheck *c, const char *model);

// {"state": "ok" | "mismatch" | "pending", "selected", "actual"}, the
// `check` update Heron's frontend reads. pending until an answer was
// judged against the current selection.
Json *model_check_verdict(const ModelCheck *c);
