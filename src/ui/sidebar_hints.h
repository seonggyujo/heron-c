// The note at the bottom of the agent sidebar (Heron's ui/agent-hints.ts
// and agent-hints.css): first an offer to turn on heron-c's Claude Code
// hooks (permission requests, questions, model checks), with its button
// and any error under it; once they are on or the offer is closed, a
// pointer to the heron-limits plugin until its data arrives. Each closes
// for good with its ×. 13 px text, 1.4 line height, Korean kept whole
// between spaces. Only ui/sidebar*.c include this.
#pragma once

#include "ui/app.h"

typedef struct {
  SidebarPart note;  // SIDEBAR_OFFER_CLOSE or SIDEBAR_PLUGIN_CLOSE, SIDEBAR_NONE for no note
  Box text, close;
  Box turn_on, error;  // the offer's; h 0 otherwise
  int h;
} HintsLayout;

// x, y, w: the space inside the foot's padding.
void sidebar_hints_layout(App *a, int x, int y, int w, HintsLayout *l);
void sidebar_hints_paint(App *a, const HintsLayout *l);
SidebarPart sidebar_hints_hit(const HintsLayout *l, int x, int y);
