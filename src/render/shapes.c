// Rectangles, rounded boxes, circles, lines and clipping on a Canvas.
#include "render/canvas_d2d.h"

void canvas_clear(Canvas *c, uint32_t argb) {
  D2D1_COLOR_F color = {((argb >> 16) & 0xFF) / 255.0f, ((argb >> 8) & 0xFF) / 255.0f,
                        (argb & 0xFF) / 255.0f, 1.0f};
  ID2D1RenderTarget_Clear(canvas_target(c), &color);
}

void canvas_fill_rect(Canvas *c, float x, float y, float w, float h, uint32_t argb) {
  D2D1_RECT_F r = {x, y, x + w, y + h};
  ID2D1RenderTarget_FillRectangle(canvas_target(c), &r, canvas_brush(c, argb));
}

void canvas_fill_round(Canvas *c, float x, float y, float w, float h, float radius, uint32_t argb) {
  D2D1_ROUNDED_RECT r = {{x, y, x + w, y + h}, radius, radius};
  ID2D1RenderTarget_FillRoundedRectangle(canvas_target(c), &r, canvas_brush(c, argb));
}

void canvas_stroke_round(Canvas *c, float x, float y, float w, float h, float radius, float width,
                         uint32_t argb) {
  float in = width / 2;
  D2D1_ROUNDED_RECT r = {{x + in, y + in, x + w - in, y + h - in}, radius - in, radius - in};
  ID2D1RenderTarget_DrawRoundedRectangle(canvas_target(c), &r, canvas_brush(c, argb), width, NULL);
}

void canvas_fill_circle(Canvas *c, float cx, float cy, float radius, uint32_t argb) {
  D2D1_ELLIPSE e = {{cx, cy}, radius, radius};
  ID2D1RenderTarget_FillEllipse(canvas_target(c), &e, canvas_brush(c, argb));
}

void canvas_stroke_circle(Canvas *c, float cx, float cy, float radius, float width, uint32_t argb) {
  D2D1_ELLIPSE e = {{cx, cy}, radius - width / 2, radius - width / 2};
  ID2D1RenderTarget_DrawEllipse(canvas_target(c), &e, canvas_brush(c, argb), width, NULL);
}

void canvas_line(Canvas *c, float x0, float y0, float x1, float y1, float width, uint32_t argb) {
  D2D1_POINT_2F a = {x0, y0}, b = {x1, y1};
  ID2D1RenderTarget_DrawLine(canvas_target(c), a, b, canvas_brush(c, argb), width, NULL);
}

void canvas_push_clip(Canvas *c, float x, float y, float w, float h) {
  D2D1_RECT_F r = {x, y, x + w, y + h};
  ID2D1RenderTarget_PushAxisAlignedClip(canvas_target(c), &r, D2D1_ANTIALIAS_MODE_ALIASED);
}

void canvas_pop_clip(Canvas *c) { ID2D1RenderTarget_PopAxisAlignedClip(canvas_target(c)); }
