// xterm.js default colors.
#include "vt/palette.h"

#include "vt/cell.h"

static const uint32_t ANSI[16] = {
    0x2e3436, 0xcc0000, 0x4e9a06, 0xc4a000, 0x3465a4, 0x75507b, 0x06989a, 0xd3d7cf,
    0x555753, 0xef2929, 0x8ae234, 0xfce94f, 0x729fcf, 0xad7fa8, 0x34e2e2, 0xeeeeec,
};

static uint32_t cube_level(int v) { return v ? (uint32_t)(55 + v * 40) : 0; }

uint32_t palette_rgb(int index) {
  if (index < 0 || index > 255) return PALETTE_FG;
  if (index < 16) return ANSI[index];
  if (index < 232) {
    int i = index - 16;
    return cube_level(i / 36) << 16 | cube_level((i / 6) % 6) << 8 | cube_level(i % 6);
  }
  uint32_t gray = (uint32_t)(8 + (index - 232) * 10);
  return gray << 16 | gray << 8 | gray;
}

uint32_t color_to_rgb(uint32_t color, bool foreground) {
  switch (COLOR_KIND(color)) {
    case COLOR_KIND_INDEX:
      return palette_rgb((int)(color & 0xFF));
    case COLOR_KIND_RGB:
      return color & 0xFFFFFF;
    default:
      return foreground ? PALETTE_FG : PALETTE_BG;
  }
}
