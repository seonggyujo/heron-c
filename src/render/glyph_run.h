// Drawing terminal characters of one color and face at cell positions.
// Characters the terminal font has go out as one glyph run with every
// advance forced to the cell grid; the others come from fallback fonts,
// centered in their cells and shrunk when they would not fit.
#pragma once

#include <stdint.h>

#include "render/canvas.h"
#include "render/mono_font_faces.h"

// cols[i] is how many cells character i takes (1 or 2). x is the left edge
// of the first cell, baseline the text baseline.
void glyph_run_draw(Canvas *c, const MonoFont *f, FaceStyle style, const uint32_t *cps,
                    const uint8_t *cols, int n, float x, float baseline, uint32_t argb);
