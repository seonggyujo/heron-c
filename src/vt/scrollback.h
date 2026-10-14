// Lines that scrolled off the top of the main screen, oldest first, in a
// ring that drops the oldest line when full.
#pragma once

#include <stdint.h>

#include "vt/cell.h"

typedef struct {
  Line *lines;
  int cap, start, count;
  // Lines ever pushed. Line i (0 = oldest kept) has id pushed - count + i,
  // and screen row y has id pushed + y, so ids survive scrolling.
  uint64_t pushed;
} Scrollback;

void scrollback_init(Scrollback *sb, int cap);
void scrollback_free(Scrollback *sb);

// Takes ownership of the line's cells.
void scrollback_push(Scrollback *sb, Line line);

// i from 0 (oldest) to count - 1 (newest).
const Line *scrollback_line(const Scrollback *sb, int i);

void scrollback_clear(Scrollback *sb);
