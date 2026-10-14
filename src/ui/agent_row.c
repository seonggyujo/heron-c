// An agent row's texts, height and tooltip.
#include "ui/agent_row.h"

#include <stdio.h>
#include <string.h>

#include "agent/model_label.h"
#include "app/strings.h"

const char *agent_state_text(AgentState state) {
  const Strings *s = strings();
  switch (state) {
    case AGENT_WORKING: return s->agent_working;
    case AGENT_PERMISSION: return s->agent_permission;
    case AGENT_QUESTION: return s->agent_question;
    case AGENT_WAITING: return s->agent_waiting;
    case AGENT_DONE: return s->agent_done;
    default: return s->agent_idle;
  }
}

void agent_line_text(const Agent *agent, char *out, size_t cap) {
  const char *state = agent_state_text(agent->state);
  bool asks = agent->state == AGENT_PERMISSION || agent->state == AGENT_QUESTION;
  if (asks && agent->message[0]) snprintf(out, cap, "%s · %s", state, agent->message);
  else snprintf(out, cap, "%s", state);
}

void agent_model_text(const Agent *agent, char *out, size_t cap) {
  // It follows /model within a poll.
  const char *model = agent->check == CHECK_NONE ? ""
                      : agent->selected[0]       ? agent->selected
                                                 : agent->actual;
  out[0] = 0;
  if (model[0]) model_label(model, out, cap);
}

int agent_sub_lines(const Agent *agent) {
  int n = agent->subagent_count;
  return n > AGENT_SUBS_SHOWN ? AGENT_SUBS_SHOWN + 1 : n;
}

int agent_row_height(const App *a, const AgentRow *r, bool collapsed) {
  if (collapsed) return app_px(a, 38);
  float h = 22 + 20 + 2 + 17;  // padding and border, top line, gap, state line
  if (r->agent->check == CHECK_MISMATCH) h += 2 + 17;
  int subs = agent_sub_lines(r->agent);
  if (subs) h += 2 + 2 + subs * 16 + (subs - 1);
  return app_px(a, h);
}

static void add_line(char *out, size_t cap, const char *line) {
  if (!line || !line[0]) return;
  size_t len = strlen(out);
  snprintf(out + len, cap - len, "%s%s", len ? "\n" : "", line);
}

void agent_row_tip(const AgentRow *r, char *out, size_t cap) {
  const Agent *agent = r->agent;
  char line[640];
  out[0] = 0;
  snprintf(line, sizeof line, "%d · %s · %s", r->number, r->folder, agent_state_text(agent->state));
  add_line(out, cap, line);
  add_line(out, cap, r->path);
  add_line(out, cap, agent->title);
  add_line(out, cap, agent->message);
  if (r->changes && r->changes->files > 0) {
    format_uncommitted(line, sizeof line, (int)r->changes->files);
    add_line(out, cap, line);
  }
  if (agent->check == CHECK_MISMATCH) {
    const char *selected = agent->selected[0] ? agent->selected : "?";
    format_agent_mismatch(line, sizeof line, selected, agent->actual[0] ? agent->actual : "?");
    add_line(out, cap, line);
  }
}
