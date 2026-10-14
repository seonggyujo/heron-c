// One row of the agent sidebar (Heron's ui/agent-item.ts, subagent-lines.ts
// and their CSS):
//   [1] heron   Opus 5.5   2m 14s
//       needs permission · Bash: cargo test     +12 −3
//       ⚠ Sonnet 5
//       ↳ Explore              1m 02s
// The badge takes the state's color; a row that needs the user gets a
// yellow border and state line. The model is the session's, the counts the
// folder's uncommitted lines, the last lines its running subagents (three,
// then "+2 more"). Collapsed, only the badge shows; the tooltip says the
// rest.
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "ui/agent_rows.h"

int agent_row_height(const App *a, const AgentRow *r, bool collapsed);
void agent_row_paint(App *a, const AgentRow *r, Box b, bool collapsed, bool hover);
void agent_row_tip(const AgentRow *r, char *out, size_t cap);

// For the painting (agent_row_paint.c).
const char *agent_state_text(AgentState state);
// The state, and what the agent asks for when it waits on a permission or
// a question.
void agent_line_text(const Agent *agent, char *out, size_t cap);
// The label of the model the session runs with, "" when not known.
void agent_model_text(const Agent *agent, char *out, size_t cap);
// Subagent lines under the row, the "+2 more" line included.
int agent_sub_lines(const Agent *agent);
#define AGENT_SUBS_SHOWN 3
