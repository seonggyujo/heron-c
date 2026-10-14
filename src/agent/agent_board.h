// The agents of all panes, keyed by pane id, and the newest usage limits
// (Heron's agent/agent-board.ts). Every signal goes through agent_model.h
// and usage.h; each call says whether the pane's agent changed and from
// which state to which. The model check and the subagents come only when
// they change, so the last of each is kept per pane and given to an agent
// that starts there again.
#pragma once

#include "agent/agent_model.h"
#include "agent/usage.h"

typedef struct {
  int pane;
  AgentSlot slot;
  Json *last_check, *last_subagents;  // what was last reported, agent or not
} BoardEntry;

typedef struct {
  BoardEntry *entries;
  int count, cap;
  bool has_limits;
  Limits limits;
  bool status_seen;  // a status file of the plugin has come, with limits or not
} AgentBoard;

// What a call did to one pane's agent.
typedef struct {
  bool changed;
  bool from_present, to_present;
  AgentState from, to;
} AgentChange;

const Agent *board_get(const AgentBoard *b, int pane);

// kind and data as pane_watch.h gives them.
AgentChange board_update(AgentBoard *b, int pane, const char *kind, const Json *data);
AgentChange board_title(AgentBoard *b, int pane, const char *title);
AgentChange board_input(AgentBoard *b, int pane, bool interrupt);
AgentChange board_seen(AgentBoard *b, int pane);
// The shell drew its prompt again: no agent there now.
AgentChange board_remove(AgentBoard *b, int pane);
// The pane closed: its agent and what was last reported for it go.
AgentChange board_forget(AgentBoard *b, int pane);
void board_free(AgentBoard *b);
