// What heron-c knows about the coding agent (Claude Code) in one pane, and
// how each signal changes it (Heron's agent/agent-model.ts). Pure. Sources:
//   - agent updates (exact, pane_watch.h): the state and the running
//     subagents from the hooks, the model check, and context use from the
//     heron-limits plugin's status line (its limits: usage.h)
//   - the terminal title (rough): "✳ <title>" idle, a turning half circle
//     working
// Once a hook has reported for a pane, hooks decide the state. Text fields
// are empty when unknown.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "base/json.h"

typedef enum { AGENT_IDLE, AGENT_WORKING, AGENT_PERMISSION, AGENT_QUESTION, AGENT_WAITING, AGENT_DONE } AgentState;
typedef enum { CHECK_NONE, CHECK_OK, CHECK_MISMATCH, CHECK_PENDING } CheckState;
// How a state looks: green while working, yellow when it needs you, blue
// when done.
typedef enum { MARK_IDLE, MARK_WORK, MARK_NEED, MARK_DONE } AgentMark;

#define AGENT_SUBAGENTS_SHOWN 8

typedef struct {
  char type[64];   // e.g. "Explore"; "agent" when Claude Code gave none
  uint64_t since;  // start, ms since 1970
} Subagent;

typedef struct {
  AgentState state;
  uint64_t since;   // when the state began, for "working 2:14"
  bool from_hooks;  // a hook has reported for this pane
  char title[256];  // the session title, from the terminal title
  char message[512];  // what it asks for, e.g. which tool needs permission
  CheckState check;
  char selected[128], actual[128];  // the models of the check
  bool has_context;
  double context;  // percent of the context window in use
  char cwd[1024];  // the agent's working folder (hooks)
  char session_id[64];
  int subagent_count;  // running now; the first AGENT_SUBAGENTS_SHOWN are kept
  Subagent subagents[AGENT_SUBAGENTS_SHOWN];
} Agent;

// One pane's agent, if it has one.
typedef struct {
  bool present;
  Agent agent;
} AgentSlot;

// The hook state file; "ended" removes the agent.
void agent_apply_state(AgentSlot *s, const Json *data, uint64_t now);
// The plugin's status file: context use.
void agent_apply_status(AgentSlot *s, const Json *data, uint64_t now);
// Only for a pane that has an agent.
void agent_apply_subagents(AgentSlot *s, const Json *data, uint64_t now);
void agent_apply_check(AgentSlot *s, const Json *data);
void agent_apply_title(AgentSlot *s, const char *title, uint64_t now);
// The user typed: an answered permission or question means work goes on.
// Esc or Ctrl+C (interrupt) stops the turn, and Claude Code runs no Stop
// hook for a stopped turn, so only the key tells the agent is idle again.
void agent_apply_input(AgentSlot *s, uint64_t now, bool interrupt);
// The pane got focus: a finished turn has been seen.
void agent_apply_seen(AgentSlot *s, uint64_t now);

// Claude Code's terminal title: false for any other title.
bool agent_read_title(const char *title, bool *working, const char **rest);
// Sort key, most urgent first: needs you, finished, working, idle.
int agent_urgency(AgentState state);
AgentMark agent_mark(AgentState state);
// Worth a taskbar flash: the agent now needs the user, or just finished.
bool agent_wants_attention(bool from_present, AgentState from, bool to_present, AgentState to);
