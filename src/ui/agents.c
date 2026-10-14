// The agent board fed by file watches and pane signals.
#include "ui/agents.h"

#include <stdlib.h>
#include <string.h>

#include "agent/agent_files.h"
#include "app/theme.h"
#include "base/log.h"
#include "base/mem.h"
#include "ui/changes.h"
#include "ui/hooks_state.h"
#include "ui/recent.h"
#include "ui/usage_bar.h"

#define POLL_MS 700
// After these flashes the button stays lit until heron-c comes to the front.
#define FLASH_COUNT 3

static void flash_taskbar(App *a) {
  if (GetForegroundWindow() == a->hwnd) return;
  FLASHWINFO f = {sizeof f, a->hwnd, FLASHW_TRAY, FLASH_COUNT, 0};
  FlashWindowEx(&f);
  log_debug("attention", "taskbar button flashed");
}

static void changed(App *a, AgentChange c) {
  if (!c.changed) return;
  changes_watch(a);
  recent_running(a);
  usage_bar_sync(a);
  app_redraw(a);
  if (agent_wants_attention(c.from_present, c.from, c.to_present, c.to)) flash_taskbar(a);
}

void agents_start(App *a) {
  const char *dir = agent_dir();
  if (dir) {
    agent_remove_stale(dir);
    log_debug("agent", "watching %s", dir);
  }
  SetTimer(a->hwnd, TIMER_AGENTS, POLL_MS, NULL);
  hooks_sync(a);
  recent_load(a);
}

static WatchedPane *watch_of(AgentsState *s, int pane) {
  for (int i = 0; i < s->watch_count; i++) {
    if (s->watches[i].pane == pane) return &s->watches[i];
  }
  if (s->watch_count == s->watch_cap) {
    s->watch_cap = s->watch_cap ? s->watch_cap * 2 : 8;
    s->watches = xrealloc(s->watches, sizeof *s->watches * (size_t)s->watch_cap);
  }
  WatchedPane *w = &s->watches[s->watch_count++];
  memset(w, 0, sizeof *w);
  w->pane = pane;
  return w;
}

static void drop_watch(AgentsState *s, int i) {
  pane_watch_free(&s->watches[i].watch);
  s->watches[i] = s->watches[--s->watch_count];
}

void agents_poll(App *a) {
  const char *dir = agent_dir();
  AgentsState *s = &a->agents;
  if (!dir) return;
  for (int i = s->watch_count - 1; i >= 0; i--) {
    Pane *p = panes_find(&a->panes, s->watches[i].pane);
    if (!p || p->closed_at) drop_watch(s, i);
  }
  for (int i = 0; i < a->panes.shown.count; i++) {
    int pane = a->panes.shown.items[i]->id;
    AgentUpdate updates[PANE_WATCH_UPDATES];
    int n = pane_watch_poll(&watch_of(s, pane)->watch, dir, pane, updates);
    for (int u = 0; u < n; u++) {
      log_debug("agent", "pane %d %s changed", pane, updates[u].kind);
      changed(a, board_update(&s->board, pane, updates[u].kind, updates[u].data));
      json_free(updates[u].data);
    }
  }
}

void agents_title(App *a, Pane *p) { changed(a, board_title(&a->agents.board, p->id, term_title(p->term))); }

void agents_prompt(App *a, Pane *p) { changed(a, board_remove(&a->agents.board, p->id)); }

void agents_key(App *a, Pane *p, bool interrupt) { changed(a, board_input(&a->agents.board, p->id, interrupt)); }

void agents_seen(App *a, Pane *p) { changed(a, board_seen(&a->agents.board, p->id)); }

void agents_closed(App *a, int pane) {
  AgentsState *s = &a->agents;
  if (a->agents.hover_pane == pane) a->agents.hover_pane = 0;
  for (int i = 0; i < s->watch_count; i++) {
    if (s->watches[i].pane == pane) {
      drop_watch(s, i);
      break;
    }
  }
  if (agent_dir()) agent_remove_pane(agent_dir(), pane);
  changed(a, board_forget(&s->board, pane));
}

const Agent *agents_of(const App *a, const Pane *p) { return board_get(&a->agents.board, p->id); }

uint32_t agents_dot_color(const App *a, const Pane *p) {
  const Agent *agent = agents_of(a, p);
  if (!agent) return 0;
  switch (agent_mark(agent->state)) {
    case MARK_WORK: return THEME_AGENT_WORK;
    case MARK_NEED: return THEME_AGENT_NEED;
    case MARK_DONE: return THEME_AGENT_DONE;
    default: return 0;
  }
}
