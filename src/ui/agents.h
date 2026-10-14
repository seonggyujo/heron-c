// The coding agents in the panes, wired like Heron's wire-agents.ts: every
// 0.7 s each shown pane's agent files are read (agent/pane_watch.h) into
// the board, together with the pane signals (title, prompt, keys, focus,
// close). A change redraws the sidebar, the usage bar and the pane dots,
// tells git and the recent sessions, and an agent that needs the user or
// just finished flashes the taskbar button while another app is in front.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void agents_start(App *a);
// TIMER_AGENTS.
void agents_poll(App *a);

void agents_title(App *a, Pane *p);
// The shell drew its prompt again: no agent runs there now.
void agents_prompt(App *a, Pane *p);
// A key went to the pane; interrupt for Esc or Ctrl+C on their own.
void agents_key(App *a, Pane *p, bool interrupt);
// The pane got focus: a finished turn has been seen.
void agents_seen(App *a, Pane *p);
// The pane closed: its agent and its files go.
void agents_closed(App *a, int pane);

const Agent *agents_of(const App *a, const Pane *p);
// The status dot color of the pane's agent state, 0 for the usual one.
uint32_t agents_dot_color(const App *a, const Pane *p);
