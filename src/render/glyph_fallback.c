// Fallback glyph lookup with a small cache keyed by code point.
#include "render/glyph_fallback.h"

#include "render/fonts.h"

static const wchar_t *const FAMILIES[] = {
    L"Malgun Gothic",   L"Segoe UI Emoji", L"Segoe UI Symbol", L"Microsoft YaHei",
    L"Yu Gothic",       L"Nirmala UI",     L"Segoe UI",        L"Cambria Math",
    L"Microsoft JhengHei",
};
#define FAMILY_COUNT ((int)(sizeof FAMILIES / sizeof FAMILIES[0]))

static IDWriteFontFace *g_faces[FAMILY_COUNT];
static bool g_loaded[FAMILY_COUNT];

typedef struct {
  uint32_t cp;   // 0: empty slot
  int8_t face;   // -1: no font has it
  UINT16 glyph;
} Slot;

#define CACHE_SIZE 4096  // a power of two
static Slot g_cache[CACHE_SIZE];

static IDWriteFontFace *face_at(int i) {
  if (!g_loaded[i]) {
    g_loaded[i] = true;
    g_faces[i] = fonts_face(FAMILIES[i], DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
  }
  return g_faces[i];
}

static Slot lookup(uint32_t cp) {
  Slot s = {cp, -1, 0};
  for (int i = 0; i < FAMILY_COUNT; i++) {
    IDWriteFontFace *face = face_at(i);
    UINT16 glyph = 0;
    if (face && SUCCEEDED(IDWriteFontFace_GetGlyphIndices(face, &cp, 1, &glyph)) && glyph) {
      s.face = (int8_t)i;
      s.glyph = glyph;
      break;
    }
  }
  return s;
}

bool glyph_fallback(uint32_t cp, IDWriteFontFace **face, UINT16 *glyph) {
  Slot *slot = &g_cache[(cp * 2654435761u) & (CACHE_SIZE - 1)];
  if (slot->cp != cp) *slot = lookup(cp);  // a miss replaces whatever was there
  if (slot->face < 0) return false;
  *face = g_faces[slot->face];
  *glyph = slot->glyph;
  return true;
}
