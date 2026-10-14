// Making, filling and resizing lines of cells.
#pragma once

#include "vt/cell.h"

// An erased cell: no character and default colors, except the background,
// which is the one in effect when erasing (xterm's background color erase).
Cell cell_blank(uint32_t bg);

void line_init(Line *l, int len, Cell blank);
void line_free(Line *l);

// Fills cells [from, to). Filling half of a wide character blanks the other
// half too, so no orphan halves are left.
void line_fill(Line *l, int from, int to, Cell blank);

// Blanks halves of wide characters whose other half is gone, after cells
// were shifted left or right.
void line_fix_wide(Line *l, Cell blank);

void line_resize(Line *l, int len, Cell blank);

// A copy without the trailing empty cells, for history.
void line_copy_trimmed(const Line *src, Line *dst);
