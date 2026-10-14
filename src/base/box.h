// A rectangle in whole pixels, used for layout and hit testing.
#pragma once

#include <stdbool.h>

typedef struct {
  int x, y, w, h;
} Box;

static inline bool box_contains(Box b, int x, int y) {
  return x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h;
}
