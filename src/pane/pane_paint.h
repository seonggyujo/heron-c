// Drawing one pane: its rounded frame, the header and the terminal. Status
// lives in the header only; the terminal area shows the program's output
// and nothing else.
#pragma once

#include <stdbool.h>

#include "pane/pane.h"
#include "render/canvas.h"
#include "render/ui_text.h"

typedef struct {
  UiFont *header_font;
  const MonoFont *font;
  const PaneMetrics *m;
  float scale;
  ULONGLONG now;  // GetTickCount64, for the uptime
} PaneStyle;

typedef struct {
  bool selected;  // the pane keyboard input goes to
  float glow;     // its focus glow, 0..1 (it fades in and out)
  bool focused;   // selected while the window is active: a solid cursor
  bool close_hover;
  bool drop_target;  // files are being dragged over it
  float flash;  // output flash on the border, 0..1
  float dot;    // status dot brightness, 0..1 (pulses while output arrives)
  uint32_t dot_color;  // the agent state's color, 0 for the usual one
  bool outlined;       // its row in the agent sidebar is under the mouse
  const char *composition;  // IME text being composed, for the focused pane
} PaneState;

void pane_paint(Canvas *c, const Pane *p, const PaneStyle *s, PaneState state);

// The header's close button.
Box pane_close_box(const Pane *p, const PaneMetrics *m);

// pane_header.c
void pane_header_paint(Canvas *c, const Pane *p, const PaneStyle *s, PaneState state);
// The header's folder or title under (x, y), whose full text shows as a
// tooltip (it may be cut short in the header).
bool pane_header_tip(const Pane *p, const PaneStyle *s, int x, int y, Box *box, const char **text);
