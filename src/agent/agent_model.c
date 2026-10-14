// Applying signals to a pane's agent.
#include "agent/agent_model.h"

#include <stdio.h>
#include <string.h>

#include "agent/usage.h"

static const char *const STATE_NAMES[] = {"idle", "working", "permission", "question", "waiting", "done"};

static void fresh(AgentSlot *s, uint64_t now) {
  memset(s, 0, sizeof *s);
  s->present = true;
  s->agent.state = AGENT_IDLE;
  s->agent.since = now;
}

static void ensure(AgentSlot *s, uint64_t now) {
  if (!s->present) fresh(s, now);
}

static void with_state(Agent *a, AgentState state, uint64_t now) {
  if (a->state == state) return;
  a->state = state;
  a->since = now;
}

// A non-empty string into out; out keeps its value otherwise when keep.
static void take_text(char *out, size_t cap, const Json *v, bool keep) {
  const char *text = json_text(v);
  if (text) snprintf(out, cap, "%s", text);
  else if (!keep) out[0] = 0;
}

void agent_apply_state(AgentSlot *s, const Json *data, uint64_t now) {
  const char *name = json_string(json_get(data, "state"));
  if (name && strcmp(name, "ended") == 0) {
    s->present = false;
    return;
  }
  int state = -1;
  for (int i = 0; name && i < 6; i++) {
    if (strcmp(name, STATE_NAMES[i]) == 0) state = i;
  }
  if (state < 0) return;
  ensure(s, now);
  Agent *a = &s->agent;
  a->from_hooks = true;
  with_state(a, (AgentState)state, now);
  take_text(a->message, sizeof a->message, json_get(data, "message"), false);
  take_text(a->cwd, sizeof a->cwd, json_get(data, "cwd"), true);
  take_text(a->session_id, sizeof a->session_id, json_get(data, "session_id"), true);
}

void agent_apply_status(AgentSlot *s, const Json *data, uint64_t now) {
  ensure(s, now);
  s->agent.has_context = usage_context_of(data, &s->agent.context);
}

void agent_apply_subagents(AgentSlot *s, const Json *data, uint64_t now) {
  if (!s->present) return;
  Agent *a = &s->agent;
  a->subagent_count = 0;
  memset(a->subagents, 0, sizeof a->subagents);
  for (int i = 0; json_is(data, JSON_ARRAY) && i < data->count; i++) {
    const Json *item = data->items[i];
    if (!json_text(json_get(item, "id"))) continue;
    if (a->subagent_count < AGENT_SUBAGENTS_SHOWN) {
      Subagent *sub = &a->subagents[a->subagent_count];
      const char *type = json_text(json_get(item, "type"));
      snprintf(sub->type, sizeof sub->type, "%s", type ? type : "agent");
      double started;
      sub->since = json_number(json_get(item, "started"), &started) ? (uint64_t)started : now;
    }
    a->subagent_count++;
  }
}

void agent_apply_check(AgentSlot *s, const Json *data) {
  if (!s->present) return;
  Agent *a = &s->agent;
  const char *state = json_string(json_get(data, "state"));
  a->check = !state ? CHECK_NONE
           : strcmp(state, "ok") == 0 ? CHECK_OK
           : strcmp(state, "mismatch") == 0 ? CHECK_MISMATCH
           : strcmp(state, "pending") == 0 ? CHECK_PENDING : CHECK_NONE;
  take_text(a->selected, sizeof a->selected, a->check ? json_get(data, "selected") : NULL, false);
  take_text(a->actual, sizeof a->actual, a->check ? json_get(data, "actual") : NULL, false);
}

// One of the four turning half circles (U+25D0..U+25D3) or ✳ (U+2733),
// then white space.
bool agent_read_title(const char *title, bool *working, const char **rest) {
  const unsigned char *t = (const unsigned char *)title;
  bool spinner = t[0] == 0xE2 && t[1] == 0x97 && t[2] >= 0x90 && t[2] <= 0x93;
  bool star = t[0] == 0xE2 && t[1] == 0x9C && t[2] == 0xB3;
  if (!spinner && !star) return false;
  const char *p = title + 3;
  if (*p != ' ' && *p != '\t') return false;
  while (*p == ' ' || *p == '\t') p++;
  *working = spinner;
  *rest = p;
  return true;
}

void agent_apply_title(AgentSlot *s, const char *title, uint64_t now) {
  bool working;
  const char *rest;
  if (!agent_read_title(title, &working, &rest)) return;
  ensure(s, now);
  Agent *a = &s->agent;
  snprintf(a->title, sizeof a->title, "%s", rest);
  if (a->from_hooks) return;
  if (working) with_state(a, AGENT_WORKING, now);
  else with_state(a, a->state == AGENT_WORKING || a->state == AGENT_DONE ? AGENT_DONE : AGENT_IDLE, now);
}

void agent_apply_input(AgentSlot *s, uint64_t now, bool interrupt) {
  if (!s->present) return;
  Agent *a = &s->agent;
  if (interrupt && (a->state == AGENT_WORKING || a->state == AGENT_PERMISSION)) {
    with_state(a, AGENT_IDLE, now);
  } else if (a->state == AGENT_PERMISSION || a->state == AGENT_QUESTION) {
    with_state(a, AGENT_WORKING, now);
  }
}

void agent_apply_seen(AgentSlot *s, uint64_t now) {
  if (s->present && (s->agent.state == AGENT_DONE || s->agent.state == AGENT_WAITING)) {
    with_state(&s->agent, AGENT_IDLE, now);
  }
}

int agent_urgency(AgentState state) {
  static const int URGENCY[] = {3, 2, 0, 0, 1, 1};  // idle, working, permission, question, waiting, done
  return URGENCY[state];
}

AgentMark agent_mark(AgentState state) {
  if (state == AGENT_WORKING) return MARK_WORK;
  if (state == AGENT_PERMISSION || state == AGENT_QUESTION) return MARK_NEED;
  if (state == AGENT_DONE || state == AGENT_WAITING) return MARK_DONE;
  return MARK_IDLE;
}

bool agent_wants_attention(bool from_present, AgentState from, bool to_present, AgentState to) {
  if (!to_present || (to != AGENT_PERMISSION && to != AGENT_QUESTION && to != AGENT_DONE)) return false;
  return !from_present || agent_mark(from) != agent_mark(to);
}
