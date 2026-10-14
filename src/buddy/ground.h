// Where a heron may stand in a terminal: runs of blank cells, a few rows
// tall, in what the terminal shows now. A blank cell has no character, the
// default background and nothing drawn on it (no inverse, underline or
// strike), and is not under the cursor. Pure: reads the terminal only.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "vt/term.h"

// Free columns [c0, c1) in the rows that end at row (the heron's feet stand
// on row's lower edge).
typedef struct {
  int row, c0, c1;
} Spot;

typedef enum {
  FIND_HOME,  // the lowest run, the rightmost one on a tie
  FIND_NEAR,  // the run closest to (col, row)
  FIND_ANY,   // the pick-th run, counting round
} FindMode;

bool ground_free(const Term *t, int col, int row);
// The run through columns [col0, col1] of the h rows ending at row; false
// when any of those cells is taken.
bool ground_run(const Term *t, int row, int h, int col0, int col1, Spot *out);
// A run at least w columns wide and h rows tall.
bool ground_find(const Term *t, int w, int h, FindMode mode, int col, int row, uint32_t pick, Spot *out);
