// The DirectWrite factory, made on first use, and font lookup.
#include <initguid.h>

#include "render/fonts.h"

#include "base/log.h"

static IDWriteFactory *g_factory;

IDWriteFactory *fonts_factory(void) {
  if (!g_factory) {
    HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &IID_IDWriteFactory,
                                     (IUnknown **)&g_factory);
    if (FAILED(hr)) log_error("fonts", "DWriteCreateFactory failed: 0x%08lx", (unsigned long)hr);
  }
  return g_factory;
}

IDWriteFontFace *fonts_face(const wchar_t *family, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style) {
  IDWriteFactory *f = fonts_factory();
  if (!f) return NULL;
  IDWriteFontCollection *fonts = NULL;
  IDWriteFontFamily *fam = NULL;
  IDWriteFont *font = NULL;
  IDWriteFontFace *face = NULL;
  UINT32 index = 0;
  BOOL exists = FALSE;
  if (SUCCEEDED(IDWriteFactory_GetSystemFontCollection(f, &fonts, FALSE)) &&
      SUCCEEDED(IDWriteFontCollection_FindFamilyName(fonts, family, &index, &exists)) && exists &&
      SUCCEEDED(IDWriteFontCollection_GetFontFamily(fonts, index, &fam)) &&
      SUCCEEDED(IDWriteFontFamily_GetFirstMatchingFont(fam, weight, DWRITE_FONT_STRETCH_NORMAL,
                                                       style, &font))) {
    IDWriteFont_CreateFontFace(font, &face);
    log_debug("fonts", "%ls: weight %d, style %d, simulations %d", family, (int)IDWriteFont_GetWeight(font),
              (int)IDWriteFont_GetStyle(font), (int)IDWriteFont_GetSimulations(font));
  }
  if (font) IDWriteFont_Release(font);
  if (fam) IDWriteFontFamily_Release(fam);
  if (fonts) IDWriteFontCollection_Release(fonts);
  return face;
}

float fonts_glyph_advance(IDWriteFontFace *face, UINT16 glyph, float em) {
  DWRITE_FONT_METRICS fm;
  DWRITE_GLYPH_METRICS gm;
  IDWriteFontFace_GetMetrics(face, &fm);
  if (FAILED(IDWriteFontFace_GetDesignGlyphMetrics(face, &glyph, 1, &gm, FALSE))) return 0;
  return gm.advanceWidth * em / fm.designUnitsPerEm;
}
