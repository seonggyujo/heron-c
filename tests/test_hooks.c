// Tests for our entries in Claude Code's settings and the records a hook
// event writes (Heron's config_tests.rs and records_tests.rs).
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "claude/claude_version.h"
#include "hooks/hook_config.h"
#include "hooks/hook_records.h"

static const char EXE[] = "C:\\Users\\me\\AppData\\Local\\heron-c\\heron-c.exe";

static Json *parse(const char *text) { return json_parse(text, strlen(text)); }

static char *compact(const Json *v) {
  Buf out = {0};
  json_write(v, &out, false);
  buf_append(&out, "", 1);
  return out.data;
}

static const char USER[] =
    "{\"model\":\"opus\",\"hooks\":{\"Stop\":[{\"hooks\":[{\"type\":\"command\",\"command\":\"say done\"}]}],"
    "\"PreToolUse\":[{\"matcher\":\"Bash\",\"hooks\":[{\"type\":\"command\",\"command\":\"check.sh\"}]}]},"
    "\"statusLine\":{\"type\":\"command\",\"command\":\"status.sh\"}}";

void test_hook_config(void) {
  Json *s = parse(USER);
  CHECK(hook_config_presence(s, EXE) == HOOKS_ABSENT);
  CHECK(hook_config_add(s, EXE) && hook_config_presence(s, EXE) == HOOKS_CURRENT);
  CHECK(json_get(json_get(s, "hooks"), "Stop")->count == 2);
  const Json *start = json_get(json_get(s, "hooks"), "SessionStart")->items[0];
  CHECK_STR(json_string(json_get(start, "matcher")), "startup|resume|clear");
  CHECK(hook_config_remove(s));
  char *back = compact(s);
  CHECK_STR(back, USER);  // the same keys in the same order
  free(back);
  hook_config_add(s, "D:\\old\\heron-c.exe");  // another exe: stale, then replaced
  CHECK(hook_config_presence(s, EXE) == HOOKS_STALE);
  hook_config_add(s, EXE);
  CHECK(hook_config_presence(s, EXE) == HOOKS_CURRENT);
  CHECK(json_get(json_get(s, "hooks"), "UserPromptSubmit")->count == 1);
  json_remove(json_get(s, "hooks"), "PostModelSwitch");  // an older install
  CHECK(hook_config_presence(s, EXE) == HOOKS_STALE);
  json_free(s);
  s = parse("{}");
  hook_config_add(s, EXE);
  hook_config_remove(s);
  back = compact(s);
  CHECK_STR(back, "{}");
  free(back);
  json_free(s);
}

void test_hook_config_leaves_others(void) {
  // Heron's own entries (heron.exe) and other programs are not ours.
  const char *text = "{\"hooks\":{\"Stop\":[{\"hooks\":[{\"type\":\"command\",\"command\":\"node\",\"args\":"
                     "[\"--agent-hook\"]},{\"type\":\"command\",\"command\":\"C:\\\\Heron\\\\heron.exe\","
                     "\"args\":[\"--agent-hook\"]}]}]}}";
  Json *s = parse(text);
  CHECK(hook_config_presence(s, EXE) == HOOKS_ABSENT);
  CHECK(!hook_config_remove(s));
  char *back = compact(s);
  CHECK_STR(back, text);
  free(back);
  json_free(s);
  s = parse("{\"hooks\":[]}");
  CHECK(!hook_config_add(s, EXE));
  json_free(s);
  s = parse("{\"hooks\":{\"Stop\":{\"not\":\"a list\"}}}");
  CHECK(!hook_config_add(s, EXE));
  json_free(s);
}

static const char *state(const char *input) {
  static char text[64];
  Json *in = parse(input), *r = hook_state_record(in, NULL);
  snprintf(text, sizeof text, "%s", r ? json_string(json_get(r, "state")) : "none");
  json_free(r);
  json_free(in);
  return text;
}

void test_hook_states(void) {
  CHECK_STR(state("{\"hook_event_name\":\"UserPromptSubmit\"}"), "working");
  CHECK_STR(state("{\"hook_event_name\":\"StopFailure\"}"), "done");
  CHECK_STR(state("{\"hook_event_name\":\"Notification\",\"notification_type\":\"idle_prompt\"}"), "waiting");
  CHECK_STR(state("{\"hook_event_name\":\"Notification\",\"notification_type\":\"auth_success\"}"), "none");
  CHECK_STR(state("{\"hook_event_name\":\"PostModelSwitch\"}"), "none");
  Json *in = parse("{\"hook_event_name\":\"PermissionRequest\",\"session_id\":\"s\",\"tool_name\":\"Bash\","
                   "\"tool_input\":{\"command\":\"ping -n 40 127.0.0.1\\necho done\"}}");
  Json *request = hook_state_record(in, NULL);
  CHECK_STR(json_string(json_get(request, "message")), "Bash: ping -n 40 127.0.0.1");
  Json *note = parse("{\"hook_event_name\":\"Notification\",\"notification_type\":\"permission_prompt\","
                     "\"session_id\":\"s\",\"message\":\"Claude needs your permission\"}");
  Json *kept = hook_state_record(note, request);  // the notice keeps the request's detail
  CHECK_STR(json_string(json_get(kept, "message")), "Bash: ping -n 40 127.0.0.1");
  char *back = compact(kept);
  CHECK_STR(back, "{\"session_id\":\"s\",\"cwd\":null,\"transcript_path\":null,\"state\":\"permission\","
                  "\"message\":\"Bash: ping -n 40 127.0.0.1\"}");
  free(back);
  json_free(kept);
  json_free(note);
  json_free(request);
  json_free(in);
}

static const char *model(const char *input) {
  static char text[64];
  Json *in = parse(input), *r = hook_model_record(in);
  const Json *m = json_get(r, "model");
  snprintf(text, sizeof text, "%s", !r ? "unchanged" : json_string(m) ? json_string(m) : "unknown");
  json_free(r);
  json_free(in);
  return text;
}

void test_hook_models_and_subagents(void) {
  CHECK_STR(model("{\"hook_event_name\":\"SessionStart\",\"source\":\"startup\",\"model\":\"claude-opus-5-5\"}"),
            "claude-opus-5-5");
  CHECK_STR(model("{\"hook_event_name\":\"SessionStart\",\"source\":\"resume\"}"), "unknown");
  CHECK_STR(model("{\"hook_event_name\":\"SessionStart\",\"source\":\"clear\"}"), "unchanged");
  CHECK_STR(model("{\"hook_event_name\":\"PostModelSwitch\",\"to_model\":\"claude-sonnet-5-5\"}"), "claude-sonnet-5-5");
  CHECK_STR(model("{\"hook_event_name\":\"Stop\",\"model\":\"x\"}"), "unchanged");
  Json *in = parse("{\"hook_event_name\":\"SubagentStart\",\"agent_id\":\"a1b2\",\"agent_type\":\"Explore\"}");
  SubagentChange c = hook_subagent_change(in, 42);
  char *back = compact(c.record);
  CHECK(c.kind == SUBAGENT_START && strcmp(c.id, "a1b2") == 0);
  CHECK_STR(back, "{\"type\":\"Explore\",\"started\":42}");
  free(back);
  json_free(c.record);
  json_free(in);
  in = parse("{\"hook_event_name\":\"SubagentStart\"}");
  CHECK(hook_subagent_change(in, 0).kind == SUBAGENT_NONE);  // no agent_id
  json_free(in);
  in = parse("{\"hook_event_name\":\"SessionEnd\"}");
  CHECK(hook_subagent_change(in, 0).kind == SUBAGENT_CLEAR);
  json_free(in);
  ClaudeVersion v;
  CHECK(claude_version_parse("2.1.296 (Claude Code)\n", &v) && v.patch == 296);
  CHECK(!claude_version_parse("'claude' is not recognized", &v));
  CHECK(claude_version_parse("2.1.100", &v) && claude_version_compare(v, (ClaudeVersion){2, 1, 139}) < 0);
}
