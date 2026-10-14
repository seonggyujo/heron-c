// Keeping each pane's agent and noticing changes.
#include "agent/agent_board.h"

#include <stdlib.h>
#include <string.h>

#include "base/clock.h"
#include "base/log.h"
#include "base/mem.h"

static const char *const NAMES[] = {"idle", "working", "permission", "question", "waiting", "done"};

static BoardEntry *find(const AgentBoard *b, int pane) {
  for (int i = 0; i < b->count; i++) {
    if (b->entries[i].pane == pane) return &b->entries[i];
  }
  return NULL;
}

static BoardEntry *entry(AgentBoard *b, int pane) {
  BoardEntry *e = find(b, pane);
  if (e) return e;
  if (b->count == b->cap) {
    b->cap = b->cap ? b->cap * 2 : 8;
    b->entries = xrealloc(b->entries, sizeof(BoardEntry) * (size_t)b->cap);
  }
  e = &b->entries[b->count++];
  memset(e, 0, sizeof *e);
  e->pane = pane;
  return e;
}

const Agent *board_get(const AgentBoard *b, int pane) {
  const BoardEntry *e = find(b, pane);
  return e && e->slot.present ? &e->slot.agent : NULL;
}

// Compares the pane's agent with before; a new agent gets the check and
// subagents last reported for its pane.
static AgentChange finish(BoardEntry *e, const AgentSlot *before, uint64_t now) {
  AgentSlot *s = &e->slot;
  if (s->present && !before->present) {
    if (e->last_check) agent_apply_check(s, e->last_check);
    if (e->last_subagents) agent_apply_subagents(s, e->last_subagents, now);
  }
  AgentChange c = {false, before->present, s->present, before->agent.state, s->agent.state};
  if (!s->present && !before->present) return c;
  c.changed = s->present != before->present || memcmp(&s->agent, &before->agent, sizeof s->agent) != 0;
  if (c.changed && (c.from_present != c.to_present || c.from != c.to)) {
    log_debug("agents", "pane %d: %s -> %s", e->pane, c.from_present ? NAMES[c.from] : "none",
              c.to_present ? NAMES[c.to] : "none");
  }
  return c;
}

static void keep(Json **slot, const Json *data) {
  json_free(*slot);
  *slot = json_copy(data);
}

AgentChange board_update(AgentBoard *b, int pane, const char *kind, const Json *data) {
  uint64_t now = clock_now_ms();
  BoardEntry *e = entry(b, pane);
  AgentSlot before = e->slot;
  if (strcmp(kind, "status") == 0) {
    b->status_seen = true;
    Limits limits;
    if (usage_limits_of(data, &limits) && (!b->has_limits || limits.at >= b->limits.at)) {
      b->limits = limits;
      b->has_limits = true;
    }
    agent_apply_status(&e->slot, data, now);
  } else if (strcmp(kind, "check") == 0) {
    keep(&e->last_check, data);
    agent_apply_check(&e->slot, data);
  } else if (strcmp(kind, "subagents") == 0) {
    keep(&e->last_subagents, data);
    agent_apply_subagents(&e->slot, data, now);
  } else {
    agent_apply_state(&e->slot, data, now);
  }
  return finish(e, &before, now);
}

AgentChange board_title(AgentBoard *b, int pane, const char *title) {
  uint64_t now = clock_now_ms();
  BoardEntry *e = entry(b, pane);
  AgentSlot before = e->slot;
  agent_apply_title(&e->slot, title, now);
  return finish(e, &before, now);
}

AgentChange board_input(AgentBoard *b, int pane, bool interrupt) {
  uint64_t now = clock_now_ms();
  BoardEntry *e = entry(b, pane);
  AgentSlot before = e->slot;
  agent_apply_input(&e->slot, now, interrupt);
  return finish(e, &before, now);
}

AgentChange board_seen(AgentBoard *b, int pane) {
  uint64_t now = clock_now_ms();
  BoardEntry *e = entry(b, pane);
  AgentSlot before = e->slot;
  agent_apply_seen(&e->slot, now);
  return finish(e, &before, now);
}

AgentChange board_remove(AgentBoard *b, int pane) {
  BoardEntry *e = entry(b, pane);
  AgentSlot before = e->slot;
  e->slot.present = false;
  return finish(e, &before, clock_now_ms());
}

AgentChange board_forget(AgentBoard *b, int pane) {
  AgentChange c = board_remove(b, pane);
  BoardEntry *e = find(b, pane);
  json_free(e->last_check);
  json_free(e->last_subagents);
  *e = b->entries[--b->count];
  return c;
}

void board_free(AgentBoard *b) {
  for (int i = 0; i < b->count; i++) {
    json_free(b->entries[i].last_check);
    json_free(b->entries[i].last_subagents);
  }
  free(b->entries);
  memset(b, 0, sizeof *b);
}
