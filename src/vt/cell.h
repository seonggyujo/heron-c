// One screen cell, one line of cells, and how a color is packed into 32 bits.
#pragma once

#include <stdbool.h>
#include <stdint.h>

// A color is the terminal default, one of the 256 palette entries, or RGB.
#define COLOR_DEFAULT 0u
#define COLOR_KIND(c) ((c) >> 24)
#define COLOR_KIND_INDEX 1u
#define COLOR_KIND_RGB 2u
#define COLOR_INDEX(i) ((COLOR_KIND_INDEX << 24) | ((uint32_t)(i) & 0xFFu))
#define COLOR_RGB(r, g, b) \
  ((COLOR_KIND_RGB << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

enum {
  ATTR_BOLD = 1 << 0,
  ATTR_DIM = 1 << 1,
  ATTR_ITALIC = 1 << 2,
  ATTR_UNDERLINE = 1 << 3,
  ATTR_BLINK = 1 << 4,
  ATTR_INVERSE = 1 << 5,
  ATTR_HIDDEN = 1 << 6,
  ATTR_STRIKE = 1 << 7,
  ATTR_WIDE = 1 << 8,       // left half of a two-column character
  ATTR_WIDE_TAIL = 1 << 9,  // right half of it; ch is 0
};

typedef struct {
  uint32_t ch;  // code point, 0 for an empty cell
  uint32_t fg, bg;
  uint16_t attr;
  uint16_t link;  // OSC 8 hyperlink: 1-based index into the terminal's link table, 0 for none
} Cell;

typedef struct {
  Cell *cells;
  int len;
  bool wrapped;  // the text runs on into the next line (soft wrap)
} Line;
