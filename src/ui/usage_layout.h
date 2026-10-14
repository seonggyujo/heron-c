// Where the usage bar's meters and chips are (Heron's usage-bar.css: 240 px
// meters with 18 px padding, chips 8 px apart after a "Context" label).
// Only ui/usage_bar*.c include this.
#pragma once

#include "ui/agent_rows.h"
#include "ui/app.h"

typedef struct {
  Box bar;
  int meter_count;
  const Limit *limits[2];  // the known ones: 5-hour, weekly
  int limit_kinds[2];      // 0: 5-hour, 1: weekly
  Box meters[2];
  Box context, label;  // h 0 when no agent has a context figure
  int chip_count;
  AgentRow rows[AGENT_ROWS_MAX];  // the agents with a context figure
  Box chips[AGENT_ROWS_MAX];
} UsageLayout;

void usage_layout(App *a, UsageLayout *l);
// "1 heron", and the context use rounded.
void usage_chip_texts(const AgentRow *r, char *name, size_t name_cap, int *percent);
