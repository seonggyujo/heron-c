// Resolving a cell's attributes into the colors and face it is drawn with.
#include "render/cell_style.h"

#include "vt/palette.h"

// Mixes b into a by amount (0 to 255) for each channel.
static uint32_t mix(uint32_t a, uint32_t b, unsigned amount) {
  uint32_t out = 0;
  for (int shift = 0; shift <= 16; shift += 8) {
    unsigned ca = (a >> shift) & 0xFF, cb = (b >> shift) & 0xFF;
    out |= ((ca * (255 - amount) + cb * amount) / 255) << shift;
  }
  return out;
}

CellStyle cell_style(const Cell *cell, bool selected) {
  uint32_t fg = cell->fg;
  if ((cell->attr & ATTR_BOLD) && COLOR_KIND(fg) == COLOR_KIND_INDEX && (fg & 0xFF) < 8) fg += 8;
  uint32_t f = color_to_rgb(fg, true), b = color_to_rgb(cell->bg, false);
  if (cell->attr & ATTR_INVERSE) {
    uint32_t t = f;
    f = b;
    b = t;
  }
  if (cell->attr & ATTR_DIM) f = mix(f, b, 128);
  if (cell->attr & ATTR_HIDDEN) f = b;
  if (selected) b = mix(b, 0xFFFFFF, 77);  // about 30% white, like xterm.js
  CellStyle s;
  s.fg = 0xFF000000u | f;
  s.bg = 0xFF000000u | b;
  bool bold = cell->attr & ATTR_BOLD, italic = cell->attr & ATTR_ITALIC;
  s.face = bold ? (italic ? FACE_BOLD_ITALIC : FACE_BOLD) : (italic ? FACE_ITALIC : FACE_REGULAR);
  s.underline = cell->attr & ATTR_UNDERLINE;
  s.strike = cell->attr & ATTR_STRIKE;
  return s;
}

uint32_t cell_default_bg(void) { return 0xFF000000u | PALETTE_BG; }
