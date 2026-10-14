// How a pane is drawn while its box changes, while it appears and while it
// goes away (Heron's layout/flip.ts and pane/pane-motion.ts). It glides
// from where it was drawn to its new box in 220 ms, grows from 0.96 while
// fading in over 220 ms, and shrinks to 0.94 while fading out over 180 ms.
// Only the drawing moves: the pane and its terminal take their new size at
// once. Settings > Animations turns all of it off.
#pragma once

#include "ui/app.h"

typedef struct {
  float x, y, w, h;  // where the pane's frame is drawn
  float opacity;
} PanePose;

// Where the shown panes are drawn, taken before their boxes change.
typedef struct {
  int count;
  int *ids;
  Box *boxes;
} MotionSnapshot;

void motion_capture(const App *a, MotionSnapshot *s);
// After the boxes changed: the panes in s that moved glide from where they
// were. Frees s.
void motion_glide(App *a, MotionSnapshot *s);
void motion_enter(App *a, Pane *p);
void motion_leave(App *a, Pane *p);

PanePose motion_pose(const App *a, const Pane *p, ULONGLONG now);
// A closed pane is done leaving and can give up its place.
bool motion_left(const App *a, const Pane *p, ULONGLONG now);
// Something still moves, so frames must keep coming.
bool motion_busy(const App *a, ULONGLONG now);
