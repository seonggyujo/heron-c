// The panes that run an agent, as the sidebar and the usage bar list them
// (Heron's AgentRow in wire-agents.ts): pane number, folder name, folder,
// agent, and the folder's uncommitted changes.
#pragma once

#include "ui/app.h"

#define AGENT_ROWS_MAX 64

typedef struct {
  int pane;
  int number;  // 1-based position among the shown panes
  char folder[256];  // "heron", "heron · fix-login", or the shell name
  const char *path;  // the agent's folder, else the shell's; NULL unknown
  const Agent *agent;
  const GitChanges *changes;  // NULL outside a repository or until git answered
} AgentRow;

// In pane order; returns how many.
int agent_rows(App *a, AgentRow *out, int cap);
// Most urgent first (needs you, finished, working, idle), then by number.
void agent_rows_by_urgency(AgentRow *rows, int count);
