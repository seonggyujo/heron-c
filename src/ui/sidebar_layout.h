// Where the parts of the agent sidebar are (Heron's agent-sidebar.css and
// new-agent.css), from the top: the 38 px head with the collapse button,
// the 36 px "New agent" split button, the agent rows (they shrink and
// scroll when the recent sessions need room), a line when no agent runs,
// the recent sessions taking the rest (at least 96 px), and the notes at
// the bottom. Collapsed, only the toggle, the "+" and the badges show.
// Only ui/sidebar*.c include this.
#pragma once

#include "ui/agent_rows.h"
#include "ui/app.h"

typedef struct {
  Box frame;  // the whole sidebar, its right border included
  bool collapsed;
  Box title, toggle;
  Box new_main, new_arrow;  // new_arrow.w is 0 when collapsed
  Box list;                 // where the agent rows show
  int row_count;
  AgentRow rows[AGENT_ROWS_MAX];  // most urgent first
  Box row_boxes[AGENT_ROWS_MAX];  // scrolled: some may be outside list
  int rows_h;                     // all rows and the gaps between them
  int scroll;                     // of the rows
  Box empty;   // "no agent runs", h 0 when hidden
  Box recent;  // the recent sessions with their head, h 0 when hidden
  Box foot;    // the notes, h 0 when hidden
} SidebarLayout;

void sidebar_layout(App *a, SidebarLayout *l);
