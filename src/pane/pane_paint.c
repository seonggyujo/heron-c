// A pane's frame and terminal area.
#include "pane/pane_paint.h"

#include "app/theme.h"
#include "render/term_paint.h"
#include "vt/palette.h"

#define TERM_BG (0xFF000000u | PALETTE_BG)

// color with its alpha scaled by amount (0..1).
static uint32_t faded(uint32_t color, float amount) {
  uint32_t alpha = (uint32_t)((color >> 24) * amount);
  return (alpha << 24) | (color & 0xFFFFFF);
}

// A soft halo around the pane: rings that grow fainter outward (the CSS
// box-shadow of Heron's focus glow and output flash).
static void halo(Canvas *c, float x, float y, float w, float h, float r, float spread, uint32_t color) {
  for (int i = 1; i <= 4; i++) {
    float d = spread * i / 4;
    canvas_stroke_round(c, x - d, y - d, w + 2 * d, h + 2 * d, r + d, spread / 4, faded(color, 1 - i / 5.0f));
  }
}

Box pane_close_box(const Pane *p, const PaneMetrics *m) {
  Box b = {p->frame.x + p->frame.w - m->close_right - m->close_size,
           p->frame.y + (m->header_h - m->close_size) / 2, m->close_size, m->close_size};
  return b;
}

void pane_paint(Canvas *c, const Pane *p, const PaneStyle *s, PaneState state) {
  const PaneMetrics *m = s->m;
  float x = (float)p->frame.x, y = (float)p->frame.y, w = (float)p->frame.w, h = (float)p->frame.h;
  float r = (float)m->radius;
  if (state.glow > 0) halo(c, x, y, w, h, r, 12 * s->scale, faded(THEME_GLOW, state.glow));
  if (state.flash > 0) halo(c, x, y, w, h, r, 8 * s->scale, faded(THEME_FLASH, state.flash * 0.3f));
  canvas_fill_round(c, x, y, w, h, r, THEME_BG_PANE);
  pane_header_paint(c, p, s, state);

  // The black terminal area below the header, rounded at the bottom only.
  float top = y + m->header_h, inner = r - 1;
  canvas_fill_round(c, x + 1, top, w - 2, h - m->header_h - 1, inner, TERM_BG);
  canvas_fill_rect(c, x + 1, top, w - 2, inner, TERM_BG);
  if (state.outlined) canvas_stroke_round(c, x - 1, y - 1, w + 2, h + 2, r + 1, 1, THEME_FG_DIM);
  uint32_t border = state.drop_target ? THEME_FG
                  : state.selected || state.outlined ? THEME_BORDER_STRONG : THEME_BORDER;
  canvas_stroke_round(c, x, y, w, h, r, 1, border);
  if (state.flash > 0) canvas_stroke_round(c, x, y, w, h, r, 1, faded(THEME_FLASH, state.flash));

  canvas_push_clip(c, (float)p->body.x, (float)p->body.y, (float)p->body.w, (float)p->body.h);
  TermView view = {p->term, s->font, &p->selection, (float)p->body.x, (float)p->body.y,
                   state.focused, state.composition, p->has_hover_link ? &p->hover_link : NULL};
  term_paint(c, &view);
  canvas_pop_clip(c);
}
