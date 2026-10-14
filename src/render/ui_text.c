// UI text with DirectWrite text formats.
#include "render/ui_text.h"

#include <stdlib.h>

#include "base/mem.h"
#include "base/utf.h"
#include "render/canvas_d2d.h"
#include "render/fonts.h"

struct UiFont {
  IDWriteTextFormat *format;
  IDWriteInlineObject *ellipsis;
};

UiFont *ui_font_new(const wchar_t *family, float px, int weight) {
  IDWriteFactory *factory = fonts_factory();
  if (!factory) return NULL;
  UiFont *f = xcalloc(1, sizeof *f);
  if (FAILED(IDWriteFactory_CreateTextFormat(factory, family, NULL, (DWRITE_FONT_WEIGHT)weight, DWRITE_FONT_STYLE_NORMAL,
                                             DWRITE_FONT_STRETCH_NORMAL, px, L"ko-kr", &f->format))) {
    free(f);
    return NULL;
  }
  IDWriteTextFormat_SetWordWrapping(f->format, DWRITE_WORD_WRAPPING_NO_WRAP);
  IDWriteTextFormat_SetParagraphAlignment(f->format, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  if (SUCCEEDED(IDWriteFactory_CreateEllipsisTrimmingSign(factory, f->format, &f->ellipsis))) {
    DWRITE_TRIMMING trimming = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
    IDWriteTextFormat_SetTrimming(f->format, &trimming, f->ellipsis);
  }
  return f;
}

void ui_font_free(UiFont *f) {
  if (!f) return;
  if (f->ellipsis) IDWriteInlineObject_Release(f->ellipsis);
  IDWriteTextFormat_Release(f->format);
  free(f);
}

float ui_text_width(UiFont *f, const char *utf8) {
  if (!f || !utf8[0]) return 0;
  wchar_t *w = utf8_to_wide(utf8, -1);
  IDWriteTextLayout *layout = NULL;
  float width = 0;
  if (SUCCEEDED(IDWriteFactory_CreateTextLayout(fonts_factory(), w, (UINT32)wcslen(w), f->format,
                                                100000.0f, 1000.0f, &layout))) {
    DWRITE_TEXT_METRICS m;
    if (SUCCEEDED(IDWriteTextLayout_GetMetrics(layout, &m))) width = m.widthIncludingTrailingWhitespace;
    IDWriteTextLayout_Release(layout);
  }
  free(w);
  return width;
}

void ui_text_draw(Canvas *c, UiFont *f, const char *utf8, float x, float y, float w, float h,
                  uint32_t argb, TextAlign align) {
  if (!f || !utf8[0] || w <= 0) return;
  static const DWRITE_TEXT_ALIGNMENT ALIGNS[] = {
      DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_TEXT_ALIGNMENT_TRAILING};
  IDWriteTextFormat_SetTextAlignment(f->format, ALIGNS[align]);
  wchar_t *text = utf8_to_wide(utf8, -1);
  D2D1_RECT_F box = {x, y, x + w, y + h};
  ID2D1RenderTarget_DrawText(canvas_target(c), text, (UINT32)wcslen(text), f->format, &box,
                             canvas_brush(c, argb), D2D1_DRAW_TEXT_OPTIONS_CLIP,
                             DWRITE_MEASURING_MODE_NATURAL);
  free(text);
}
