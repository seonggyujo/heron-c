// Hook event to state, model and subagent records.
#include "hooks/hook_records.h"

#include <stdio.h>
#include <string.h>

// Longest command or path kept in a permission message, in characters.
#define DETAIL_CHARS 80

static bool is(const char *a, const char *b) { return a && strcmp(a, b) == 0; }

static const char *event_of(const Json *input) { return json_string(json_get(input, "hook_event_name")); }

// The state an event stands for, or NULL when it changes nothing.
static const char *state_of(const Json *input) {
  const char *event = event_of(input);
  if (is(event, "SessionStart")) return "idle";
  if (is(event, "UserPromptSubmit")) return "working";
  if (is(event, "PermissionRequest")) return "permission";
  if (is(event, "Stop") || is(event, "StopFailure")) return "done";
  if (is(event, "SessionEnd")) return "ended";
  if (!is(event, "Notification")) return NULL;
  const char *type = json_string(json_get(input, "notification_type"));
  if (is(type, "permission_prompt")) return "permission";
  if (is(type, "idle_prompt")) return "waiting";
  if (is(type, "elicitation_dialog")) return "question";
  return NULL;
}

// The first line of text, cut to DETAIL_CHARS characters and an ellipsis.
static void shorten(char *out, size_t cap, const char *text) {
  size_t end = strcspn(text, "\r\n"), at = 0;
  int chars = 0;
  while (at < end && chars < DETAIL_CHARS) {
    at++;
    while (at < end && ((unsigned char)text[at] & 0xC0) == 0x80) at++;  // the rest of a UTF-8 character
    chars++;
  }
  snprintf(out, cap, "%.*s%s", (int)at, text, at < end ? "\xe2\x80\xa6" : "");
}

// The notice text, or the tool asking for permission with its command,
// file or address.
static Json *message_of(const Json *input) {
  const char *event = event_of(input);
  if (is(event, "Notification")) {
    const char *message = json_string(json_get(input, "message"));
    return message ? json_new_string(message) : NULL;
  }
  if (!is(event, "PermissionRequest")) return NULL;
  const char *tool = json_string(json_get(input, "tool_name"));
  if (!tool) return NULL;
  const Json *args = json_get(input, "tool_input");
  const char *keys[] = {"command", "file_path", "url"}, *detail = NULL;
  for (int i = 0; i < 3 && !detail; i++) detail = json_string(json_get(args, keys[i]));
  if (!detail) return json_new_string(tool);
  char shortened[DETAIL_CHARS * 4 + 8], text[sizeof shortened + 256];
  shorten(shortened, sizeof shortened, detail);
  snprintf(text, sizeof text, "%s: %s", tool, shortened);
  return json_new_string(text);
}

static Json *kept_permission_detail(const Json *input, const Json *previous) {
  if (!previous || !is(json_string(json_get(input, "notification_type")), "permission_prompt")) return NULL;
  bool same_prompt = is(json_string(json_get(previous, "state")), "permission") &&
                     json_equal(json_get(previous, "session_id"), json_get(input, "session_id"));
  const char *message = same_prompt ? json_string(json_get(previous, "message")) : NULL;
  return message ? json_new_string(message) : NULL;
}

Json *hook_state_record(const Json *input, const Json *previous) {
  const char *state = state_of(input);
  if (!state) return NULL;
  Json *message = kept_permission_detail(input, previous);
  if (!message) message = message_of(input);
  Json *r = json_new(JSON_OBJECT);
  json_set_copy(r, "session_id", json_get(input, "session_id"));
  json_set_copy(r, "cwd", json_get(input, "cwd"));
  json_set_copy(r, "transcript_path", json_get(input, "transcript_path"));
  json_set(r, "state", json_new_string(state));
  json_set(r, "message", message ? message : json_new(JSON_NULL));
  return r;
}

Json *hook_model_record(const Json *input) {
  const char *event = event_of(input);
  bool start = is(event, "SessionStart");
  if (!start && !is(event, "PostModelSwitch")) return NULL;
  const char *model = json_text(json_get(input, start ? "model" : "to_model"));
  bool cleared = is(json_string(json_get(input, "source")), "clear");
  if (!model && (!start || cleared)) return NULL;
  Json *r = json_new(JSON_OBJECT);
  json_set_copy(r, "session_id", json_get(input, "session_id"));
  json_set(r, "model", model ? json_new_string(model) : json_new(JSON_NULL));
  return r;
}

SubagentChange hook_subagent_change(const Json *input, uint64_t now) {
  SubagentChange c = {SUBAGENT_NONE, NULL, NULL};
  const char *event = event_of(input), *id = json_text(json_get(input, "agent_id"));
  if (is(event, "SessionStart") || is(event, "SessionEnd")) {
    c.kind = SUBAGENT_CLEAR;
  } else if (is(event, "SubagentStart") && id) {
    c = (SubagentChange){SUBAGENT_START, id, json_new(JSON_OBJECT)};
    json_set_copy(c.record, "type", json_get(input, "agent_type"));
    json_set(c.record, "started", json_new_number((double)now));
  } else if (is(event, "SubagentStop") && id) {
    c = (SubagentChange){SUBAGENT_STOP, id, NULL};
  }
  return c;
}
