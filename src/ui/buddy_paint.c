// Turning sprite letters into colored squares.
#include "ui/buddy_paint.h"

#include <math.h>

#include "app/theme.h"

#define EYE 0xFF1B2420u
#define BEAK 0xFFE2B04Au
#define DOWN 0xFFF2D46Bu  // a chick
#define SPARK 0xFFFFE9A0u

// The letters of buddy/sprites.h.
static uint32_t ink(char letter) {
  switch (letter) {
    case 'W': return THEME_FG_SOFT;
    case 'G': return THEME_FG_DIM;
    case 'K': return EYE;
    case 'Y': return BEAK;
    case 'C': return DOWN;
    case 'N': return THEME_AGENT_NEED;
    case 'Z': return THEME_FG_DIM;
    case 'S': return SPARK;
    default: return 0;
  }
}

static uint32_t faded(uint32_t argb, float opacity) {
  uint32_t alpha = (uint32_t)((float)(argb >> 24) * opacity + 0.5f);
  return (alpha << 24) | (argb & 0xFFFFFF);
}

static void paint_item(Canvas *c, const PoseItem *it, float x, float y, float px) {
  const Sprite *s = sprite_get(it->sprite);
  float left = x + it->x * px, top = y + it->y * px;
  for (int r = 0; r < s->h; r++) {
    const char *row = s->rows[r];
    float y0 = roundf(top + (float)r * px), y1 = roundf(top + (float)(r + 1) * px);
    int v = 0;  // the column as drawn, mirrored when the sprite faces left
    while (v < s->w) {
      char letter = row[it->flip ? s->w - 1 - v : v];
      int start = v;
      while (v < s->w && row[it->flip ? s->w - 1 - v : v] == letter) v++;
      if (letter == '.') continue;
      float x0 = roundf(left + (float)start * px), x1 = roundf(left + (float)v * px);
      canvas_fill_rect(c, x0, y0, x1 - x0, y1 - y0, faded(ink(letter), it->opacity));
    }
  }
}

void buddy_paint_pose(Canvas *c, const Pose *pose, float x, float y, float px) {
  for (int i = 0; i < pose->count; i++) paint_item(c, &pose->items[i], x, y, px);
}
