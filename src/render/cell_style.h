// How a cell is drawn: bold brightens the 8 basic colors (as xterm.js does
// by default), inverse swaps foreground and background, dim fades the text,
// hidden hides it, and a selected cell gets a light overlay.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "render/mono_font_faces.h"
#include "vt/cell.h"

typedef struct {
  uint32_t fg, bg;  // 0xFFRRGGBB
  FaceStyle face;
  bool underline, strike;
} CellStyle;

CellStyle cell_style(const Cell *cell, bool selected);

// The terminal background as drawn, to skip filling cells that match it.
uint32_t cell_default_bg(void);
