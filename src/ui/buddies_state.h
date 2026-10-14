// The terminal herons the window keeps, data only (App holds one): a crew
// (buddy/crew.h) for each pane with a coding agent. ui/buddies.c keeps it.
#pragma once

#include <stdbool.h>

#include "buddy/crew.h"

typedef struct {
  int pane;
  Crew crew;
} PaneCrew;

typedef struct {
  bool on;  // Settings > Terminal herons
  PaneCrew *crews;
  int count, cap;
} BuddiesState;
