// Drawing a terminal's visible rows: backgrounds, text, underlines, the
// selection, the cursor and the IME text being composed. The caller fills
// the area with the terminal background first.
#pragma once

#include <stdbool.h>

#include "render/canvas.h"
#include "render/mono_font.h"
#include "vt/selection.h"
#include "vt/term.h"

typedef struct {
  const Term *term;
  const MonoFont *font;
  const Selection *selection;
  float x, y;               // top-left corner of cell (0, 0)
  bool focused;             // a hollow cursor when not
  const char *composition;  // IME text being composed (UTF-8), or NULL
  const TermLink *link;     // the link under the mouse, underlined, or NULL
} TermView;

void term_paint(Canvas *c, const TermView *v);
