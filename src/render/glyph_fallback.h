// Fonts to borrow a glyph from when the terminal font lacks a character
// (Hangul, CJK, emoji, symbols), tried in a fixed order. Lookups are cached.
// Only render/ includes this.
#pragma once

#include <dwrite.h>
#include <stdbool.h>
#include <stdint.h>

// The first fallback face with a glyph for cp. The face stays owned here.
bool glyph_fallback(uint32_t cp, IDWriteFontFace **face, UINT16 *glyph);
