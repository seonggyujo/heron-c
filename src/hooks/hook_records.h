// What one Claude Code hook event writes for its pane (Heron's
// claude_hooks/records.rs), pure: the agent state for the state file, the
// session's model for the model file, and the subagent files. Records are
// JSON in Heron's format, so either app's hook and the heron-limits
// plugin can share a folder.
#pragma once

#include <stdint.h>

#include "base/json.h"

// The state record, or NULL when the event changes no state. previous is
// the pane's record written just before (NULL if none): a permission
// notice that follows a PermissionRequest keeps the request's tool and
// command.
Json *hook_state_record(const Json *input, const Json *previous);

// The model record for SessionStart and PostModelSwitch, else NULL. A
// SessionStart without a model (a resumed session) records it as unknown
// (null); after /clear the model stays as it was.
Json *hook_model_record(const Json *input);

typedef enum { SUBAGENT_NONE, SUBAGENT_START, SUBAGENT_STOP, SUBAGENT_CLEAR } SubagentChangeKind;

typedef struct {
  SubagentChangeKind kind;
  const char *id;  // START and STOP: points into input
  Json *record;    // START: {type, started}, owned by the caller
} SubagentChange;

// now is ms since 1970, a starting subagent's start.
SubagentChange hook_subagent_change(const Json *input, uint64_t now);
