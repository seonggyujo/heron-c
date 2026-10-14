// The MonoFont struct: the four faces and the cell metrics. Only render/
// includes this.
#pragma once

#include <dwrite.h>

#include "render/mono_font.h"

typedef enum { FACE_REGULAR, FACE_BOLD, FACE_ITALIC, FACE_BOLD_ITALIC, FACE_COUNT } FaceStyle;

struct MonoFont {
  IDWriteFontFace *faces[FACE_COUNT];
  float em;  // font size in pixels
  int cell_w, cell_h;
  float baseline;  // from the cell top
  float underline_y, underline_h, strike_y;
};
