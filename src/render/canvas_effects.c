// Triangles, dashed strokes, fading edges, transforms and translucent
// drawing on a Canvas.
#include <float.h>
#include <string.h>

#include "render/canvas_d2d.h"

void canvas_stroke_round_dashed(Canvas *c, float x, float y, float w, float h, float radius, float width,
                                uint32_t argb) {
  float in = width / 2;
  D2D1_ROUNDED_RECT r = {{x + in, y + in, x + w - in, y + h - in}, radius - in, radius - in};
  ID2D1RenderTarget_DrawRoundedRectangle(canvas_target(c), &r, canvas_brush(c, argb), width,
                                         canvas_dash_style(c));
}

void canvas_fill_triangle(Canvas *c, const float points[6], uint32_t argb) {
  ID2D1PathGeometry *path = NULL;
  ID2D1GeometrySink *sink = NULL;
  if (FAILED(ID2D1Factory_CreatePathGeometry(canvas_factory(c), &path))) return;
  if (SUCCEEDED(ID2D1PathGeometry_Open(path, &sink))) {
    D2D1_POINT_2F start = {points[0], points[1]};
    D2D1_POINT_2F rest[2] = {{points[2], points[3]}, {points[4], points[5]}};
    ID2D1GeometrySink_BeginFigure(sink, start, D2D1_FIGURE_BEGIN_FILLED);
    ID2D1GeometrySink_AddLines(sink, rest, 2);
    ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_CLOSED);
    ID2D1GeometrySink_Close(sink);
    ID2D1GeometrySink_Release(sink);
    ID2D1RenderTarget_FillGeometry(canvas_target(c), (ID2D1Geometry *)path, canvas_brush(c, argb), NULL);
  }
  ID2D1PathGeometry_Release(path);
}

void canvas_fill_fade(Canvas *c, float x, float y, float w, float h, uint32_t argb, bool solid_top,
                      float solid_part) {
  D2D1_COLOR_F solid = canvas_color(argb), clear = solid;
  clear.a = 0;
  D2D1_GRADIENT_STOP stops[3] = {{0, solid}, {solid_part, solid}, {1, clear}};
  if (!solid_top) {
    D2D1_GRADIENT_STOP up[3] = {{0, clear}, {1 - solid_part, solid}, {1, solid}};
    memcpy(stops, up, sizeof stops);
  }
  ID2D1RenderTarget *t = canvas_target(c);
  ID2D1GradientStopCollection *collection = NULL;
  ID2D1LinearGradientBrush *brush = NULL;
  // Through the vtable: MinGW's macro for this method has the C++
  // overload's three arguments.
  if (FAILED(t->lpVtbl->CreateGradientStopCollection(t, stops, 3, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP,
                                                     &collection))) {
    return;
  }
  D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES line = {{x, y}, {x, y + h}};
  if (SUCCEEDED(ID2D1RenderTarget_CreateLinearGradientBrush(t, &line, NULL, collection, &brush))) {
    D2D1_RECT_F r = {x, y, x + w, y + h};
    ID2D1RenderTarget_FillRectangle(t, &r, (ID2D1Brush *)brush);
    ID2D1LinearGradientBrush_Release(brush);
  }
  ID2D1GradientStopCollection_Release(collection);
}

void canvas_set_transform(Canvas *c, float sx, float sy, float dx, float dy) {
  D2D1_MATRIX_3X2_F m = {{{sx, 0, 0, sy, dx, dy}}};
  ID2D1RenderTarget_SetTransform(canvas_target(c), &m);
}

void canvas_push_opacity(Canvas *c, float opacity) {
  D2D1_LAYER_PARAMETERS params = {{-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX},
                                  NULL,
                                  D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                                  {{{1, 0, 0, 1, 0, 0}}},
                                  opacity,
                                  NULL,
                                  D2D1_LAYER_OPTIONS_NONE};
  ID2D1RenderTarget_PushLayer(canvas_target(c), &params, canvas_layer(c));
}

void canvas_pop_opacity(Canvas *c) { ID2D1RenderTarget_PopLayer(canvas_target(c)); }
