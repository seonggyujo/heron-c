// The terminal herons the window keeps, data only (App holds one): a flock
// of crews (buddy/crew.h), one per heron, for each pane with a coding agent.
// ui/buddies.c keeps it.
#pragma once

#define BUDDIES_MAX 3  // herons a pane can have

#include "buddy/crew.h"

typedef struct {
  int pane;
  int count;  // crews in use; those past the wanted number are leaving
  Crew crews[BUDDIES_MAX];
} PaneFlock;

typedef struct {
  int herons;  // Settings > Terminal herons: how many per pane, 0 for off
  PaneFlock *flocks;
  int count, cap;
} BuddiesState;
