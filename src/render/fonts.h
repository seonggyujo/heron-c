// The shared DirectWrite factory and finding installed fonts by family name.
// Only render/ includes this.
#pragma once

#include <dwrite.h>

IDWriteFactory *fonts_factory(void);

// The installed face closest to the weight and style, or NULL when the
// family is not installed. The caller releases it.
IDWriteFontFace *fonts_face(const wchar_t *family, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style);

// Advance width of one glyph in pixels at em size em.
float fonts_glyph_advance(IDWriteFontFace *face, UINT16 glyph, float em);
