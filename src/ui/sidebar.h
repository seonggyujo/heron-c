// The agent sidebar left of the workspace (Heron's ui/agent-sidebar.ts):
// the "New agent" button, the panes that run an agent, most urgent first,
// the recent Claude Code sessions, and notes at the bottom. It collapses to
// a 48 px rail of numbered badges, its width gliding over 160 ms while the
// panes glide to their new boxes; the choice is remembered. Parts:
// sidebar_layout (where things are), sidebar_paint, agent_row,
// sidebar_recent, sidebar_hints, sidebar_input (the mouse).
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "input/mouse_report.h"
#include "ui/app.h"

// Reads the remembered choices: collapsed, closed notes, folded folders.
void sidebar_load(App *a);
// Settings > Reset forgot them: back to an open sidebar.
void sidebar_reset(App *a);

// The width it settles at, which the workspace leaves free, and the width
// it is drawn at now.
int sidebar_target_width(const App *a);
int sidebar_width(const App *a);
void sidebar_toggle(App *a);
// The width still moves, so frames must keep coming.
bool sidebar_busy(const App *a, ULONGLONG now);
// TIMER_CLOCK: working times count up, and a delete failure note goes.
void sidebar_tick(App *a);

void sidebar_paint(App *a);

// sidebar_input.c. Each returns true when (x, y) is on the sidebar, or the
// sidebar owns the press.
bool sidebar_mouse_move(App *a, int x, int y);
void sidebar_mouse_leave(App *a);
bool sidebar_mouse_down(App *a, MouseButton b, int x, int y);
bool sidebar_mouse_up(App *a, int x, int y);
bool sidebar_wheel(App *a, int notches, int x, int y);
// Esc closes the delete question.
bool sidebar_key(App *a, WPARAM vk);
// What the thing under (x, y) explains, written into buf; NULL for nothing.
const char *sidebar_tip_at(App *a, int x, int y, Box *box, char *buf, size_t cap);
// The "New agent" split button, which the folder menu hangs under, and
// whether (x, y) is on its ▾.
Box sidebar_new_agent_box(App *a);
bool sidebar_menu_button_at(App *a, int x, int y);
