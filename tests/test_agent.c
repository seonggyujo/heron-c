// Tests for the agent model, the board, usage and model labels.
#include <string.h>

#include "agent/agent_board.h"
#include "agent/model_label.h"
#include "check.h"

static Json *parse(const char *text) { return json_parse(text, strlen(text)); }

static AgentChange update(AgentBoard *b, int pane, const char *kind, const char *data) {
  Json *v = parse(data);
  AgentChange c = board_update(b, pane, kind, v);
  json_free(v);
  return c;
}

void test_agent_titles(void) {
  AgentBoard b = {0};
  // No hooks: the title tells. A turning half circle works, ✳ is idle.
  AgentChange c = board_title(&b, 1, "\xe2\x97\x90 Fix the login");
  CHECK(c.changed && !c.from_present && c.to_present && c.to == AGENT_WORKING);
  CHECK_STR(board_get(&b, 1)->title, "Fix the login");
  c = board_title(&b, 1, "\xe2\x9c\xb3 Fix the login");
  CHECK(c.to == AGENT_DONE);  // it was working: done
  CHECK(agent_wants_attention(c.from_present, c.from, c.to_present, c.to));
  c = board_seen(&b, 1);
  CHECK(c.to == AGENT_IDLE);
  CHECK(!board_title(&b, 1, "PowerShell").changed);  // not Claude Code's title
  c = board_remove(&b, 1);  // the shell prompt came back
  CHECK(c.changed && !c.to_present && board_get(&b, 1) == NULL);
  board_free(&b);
}

void test_agent_hooks(void) {
  AgentBoard b = {0};
  update(&b, 2, "check", "{\"state\":\"mismatch\",\"selected\":\"claude-opus-5-5\",\"actual\":\"claude-sonnet-5-5\"}");
  CHECK(board_get(&b, 2) == NULL);  // kept for an agent to come
  AgentChange c = update(&b, 2, "state",
                         "{\"state\":\"permission\",\"message\":\"Bash: npm test\",\"cwd\":\"C:\\\\w\",\"session_id\":\"s\"}");
  const Agent *a = board_get(&b, 2);
  CHECK(c.to == AGENT_PERMISSION && a->from_hooks && a->check == CHECK_MISMATCH);
  CHECK_STR(a->message, "Bash: npm test");
  CHECK_STR(a->cwd, "C:\\w");
  CHECK(board_title(&b, 2, "\xe2\x9c\xb3 x").to == AGENT_PERMISSION);  // hooks decide now
  CHECK(board_input(&b, 2, false).to == AGENT_WORKING);  // the permission was answered
  CHECK(board_input(&b, 2, true).to == AGENT_IDLE);  // Esc stopped the turn
  update(&b, 2, "subagents", "[{\"id\":\"a\",\"type\":\"Explore\",\"started\":5},{\"id\":\"b\"}]");
  CHECK(a->subagent_count == 2 && strcmp(a->subagents[1].type, "agent") == 0 && a->subagents[0].since == 5);
  c = update(&b, 2, "state", "{\"state\":\"ended\"}");
  CHECK(!c.to_present);
  update(&b, 2, "state", "{\"state\":\"working\"}");  // a new session in the pane gets the last reports
  CHECK(board_get(&b, 2)->subagent_count == 2 && board_get(&b, 2)->check == CHECK_MISMATCH);
  board_forget(&b, 2);
  CHECK(board_get(&b, 2) == NULL && b.count == 0);
  board_free(&b);
}

void test_agent_usage(void) {
  AgentBoard b = {0};
  update(&b, 3, "status", "{\"at\":100,\"context\":58.5,\"limits\":{\"five_hour\":{\"used\":71,\"resets_at\":9}}}");
  CHECK(b.status_seen && b.has_limits && b.limits.five_hour.used == 71 && !b.limits.seven_day.known);
  CHECK(board_get(&b, 3)->has_context && board_get(&b, 3)->context == 58.5);
  update(&b, 4, "status", "{\"at\":200,\"context\":null,\"limits\":{}}");  // no limits: the last stay
  CHECK(b.limits.at == 100 && !board_get(&b, 4)->has_context);
  CHECK(usage_fill(69) == FILL_OK && usage_fill(70) == FILL_WARN && usage_fill(90) == FILL_BAD);
  board_free(&b);
  char label[64];
  model_label("claude-opus-5-5", label, sizeof label);
  CHECK_STR(label, "Opus 5.5");
  model_label("claude-sonnet-5-5[1m]", label, sizeof label);
  CHECK_STR(label, "Sonnet 5.5 1M");
  model_label("claude-haiku-5-5-20260301", label, sizeof label);
  CHECK_STR(label, "Haiku 5.5");
  model_label("claude-opus-4-20250514", label, sizeof label);
  CHECK_STR(label, "Opus 4");
  model_label("opus", label, sizeof label);
  CHECK_STR(label, "Opus");
  model_label("claude-opus-4-5[1m]x", label, sizeof label);
  CHECK_STR(label, "Opus-4-5[1m]x");
}
