// Loading the terminal font and measuring its cell.
#include <math.h>
#include <stdlib.h>

#include "base/log.h"
#include "base/mem.h"
#include "render/fonts.h"
#include "render/mono_font_faces.h"

static const wchar_t *const FAMILIES[] = {L"Cascadia Mono", L"Consolas"};

// A face for weight and style, or another reference to fallback.
static IDWriteFontFace *face_or(const wchar_t *family, DWRITE_FONT_WEIGHT weight,
                                DWRITE_FONT_STYLE style, IDWriteFontFace *fallback) {
  IDWriteFontFace *face = fonts_face(family, weight, style);
  if (face) return face;
  IDWriteFontFace_AddRef(fallback);
  return fallback;
}

// Cell width is the advance of 'M' rounded to whole pixels; height is ascent
// plus descent (line height 1, as xterm.js), with the baseline centered.
static void measure(MonoFont *f) {
  IDWriteFontFace *face = f->faces[FACE_REGULAR];
  DWRITE_FONT_METRICS m;
  IDWriteFontFace_GetMetrics(face, &m);
  float scale = f->em / m.designUnitsPerEm;
  UINT32 cp = 'M';
  UINT16 glyph = 0;
  IDWriteFontFace_GetGlyphIndices(face, &cp, 1, &glyph);
  float advance = fonts_glyph_advance(face, glyph, f->em);
  float height = (m.ascent + m.descent) * scale;
  f->cell_w = advance >= 1 ? (int)lroundf(advance) : 1;
  f->cell_h = (int)ceilf(height);
  f->baseline = roundf((f->cell_h - height) / 2 + m.ascent * scale);
  f->underline_y = roundf(f->baseline - m.underlinePosition * scale);
  f->underline_h = fmaxf(1, roundf(m.underlineThickness * scale));
  f->strike_y = roundf(f->baseline - m.strikethroughPosition * scale);
}

MonoFont *mono_font_new(float px) {
  const wchar_t *family = NULL;
  IDWriteFontFace *regular = NULL;
  for (size_t i = 0; i < sizeof FAMILIES / sizeof FAMILIES[0] && !regular; i++) {
    family = FAMILIES[i];
    regular = fonts_face(family, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
  }
  if (!regular) {
    log_error("font", "neither Cascadia Mono nor Consolas is installed");
    return NULL;
  }
  MonoFont *f = xcalloc(1, sizeof *f);
  f->em = px;
  f->faces[FACE_REGULAR] = regular;
  f->faces[FACE_BOLD] = face_or(family, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, regular);
  f->faces[FACE_ITALIC] = face_or(family, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_ITALIC, regular);
  f->faces[FACE_BOLD_ITALIC] =
      face_or(family, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_ITALIC, f->faces[FACE_BOLD]);
  measure(f);
  log_debug("font", "%ls %.1fpx: cell %dx%d", family, px, f->cell_w, f->cell_h);
  return f;
}

void mono_font_free(MonoFont *f) {
  if (!f) return;
  for (int i = 0; i < FACE_COUNT; i++) IDWriteFontFace_Release(f->faces[i]);
  free(f);
}

int mono_font_cell_w(const MonoFont *f) { return f->cell_w; }
int mono_font_cell_h(const MonoFont *f) { return f->cell_h; }
