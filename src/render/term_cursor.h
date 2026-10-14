// The cursor and the IME text being composed, drawn over the terminal rows.
// Only render/term_paint.c includes this.
#pragma once

#include "render/term_paint.h"

void term_paint_cursor(Canvas *c, const TermView *v);
