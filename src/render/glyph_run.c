// Glyph runs on the cell grid, with per-character fallback fonts.
#include "render/glyph_run.h"

#include "render/canvas_d2d.h"
#include "render/fonts.h"
#include "render/glyph_fallback.h"

#define CHUNK 512

static void draw_run(Canvas *c, IDWriteFontFace *face, float em, const UINT16 *glyphs,
                     const FLOAT *advances, int n, float x, float baseline, uint32_t argb) {
  DWRITE_GLYPH_RUN run = {face, em, (UINT32)n, glyphs, advances, NULL, FALSE, 0};
  D2D1_POINT_2F origin = {x, baseline};
  ID2D1RenderTarget_DrawGlyphRun(canvas_target(c), origin, &run, canvas_brush(c, argb),
                                 DWRITE_MEASURING_MODE_NATURAL);
}

static void draw_fallback(Canvas *c, const MonoFont *f, uint32_t cp, float box_w, float x,
                          float baseline, uint32_t argb) {
  IDWriteFontFace *face;
  UINT16 glyph;
  if (!glyph_fallback(cp, &face, &glyph)) return;
  float em = f->em;
  float advance = fonts_glyph_advance(face, glyph, em);
  if (advance > box_w && advance > 0) {
    em *= box_w / advance;
    advance = box_w;
  }
  FLOAT adv = advance;
  draw_run(c, face, em, &glyph, &adv, 1, x + (box_w - advance) / 2, baseline, argb);
}

static void draw_chunk(Canvas *c, const MonoFont *f, FaceStyle style, const uint32_t *cps,
                       const uint8_t *cols, int n, float x, float baseline, uint32_t argb) {
  IDWriteFontFace *face = f->faces[style];
  UINT16 glyphs[CHUNK];
  FLOAT advances[CHUNK];
  if (FAILED(IDWriteFontFace_GetGlyphIndices(face, cps, (UINT32)n, glyphs))) return;
  int i = 0;
  while (i < n) {
    float start_x = x;
    int start = i;
    while (i < n && glyphs[i]) {
      advances[i] = (FLOAT)(cols[i] * f->cell_w);
      x += advances[i];
      i++;
    }
    if (i > start) draw_run(c, face, f->em, glyphs + start, advances + start, i - start, start_x, baseline, argb);
    while (i < n && !glyphs[i]) {
      float box_w = (float)(cols[i] * f->cell_w);
      draw_fallback(c, f, cps[i], box_w, x, baseline, argb);
      x += box_w;
      i++;
    }
  }
}

void glyph_run_draw(Canvas *c, const MonoFont *f, FaceStyle style, const uint32_t *cps,
                    const uint8_t *cols, int n, float x, float baseline, uint32_t argb) {
  for (int at = 0; at < n; at += CHUNK) {
    int len = n - at < CHUNK ? n - at : CHUNK;
    draw_chunk(c, f, style, cps + at, cols + at, len, x, baseline, argb);
    for (int i = at; i < at + len; i++) x += (float)(cols[i] * f->cell_w);
  }
}
