// Building the agent rows.
#include "ui/agent_rows.h"

#include <stdio.h>
#include <stdlib.h>

#include "app/paths.h"
#include "ui/agents.h"
#include "ui/changes.h"

int agent_rows(App *a, AgentRow *out, int cap) {
  int n = 0;
  for (int i = 0; i < a->panes.shown.count && n < cap; i++) {
    Pane *p = a->panes.shown.items[i];
    const Agent *agent = agents_of(a, p);
    if (!agent) continue;
    AgentRow *r = &out[n++];
    r->pane = p->id;
    r->number = i + 1;
    r->agent = agent;
    // The agent's own folder when the hooks report it, else the shell's.
    r->path = agent->cwd[0] ? agent->cwd : pane_path(p);
    r->folder[0] = 0;
    if (r->path) folder_label(r->path, r->folder, sizeof r->folder);
    if (!r->folder[0]) snprintf(r->folder, sizeof r->folder, "%s", shell_name(p->shell));
    r->changes = r->path ? changes_of(a, r->path) : NULL;
  }
  return n;
}

static int urgent_first(const void *x, const void *y) {
  const AgentRow *a = x, *b = y;
  int d = agent_urgency(a->agent->state) - agent_urgency(b->agent->state);
  return d ? d : a->number - b->number;
}

void agent_rows_by_urgency(AgentRow *rows, int count) { qsort(rows, (size_t)count, sizeof *rows, urgent_first); }
