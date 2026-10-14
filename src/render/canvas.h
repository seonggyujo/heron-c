// Drawing into a window with Direct2D. Coordinates are physical pixels (the
// render target runs at 96 DPI; the UI scales its own sizes). Colors are
// 0xAARRGGBB. Shapes are anti-aliased.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

typedef struct Canvas Canvas;

Canvas *canvas_new(HWND hwnd);
void canvas_free(Canvas *c);
void canvas_resize(Canvas *c, int width, int height);

// Every frame is begin, draw, end. begin returns false when there is
// nothing to draw on (for example while the window is minimized).
bool canvas_begin(Canvas *c);
void canvas_end(Canvas *c);

void canvas_clear(Canvas *c, uint32_t argb);
void canvas_fill_rect(Canvas *c, float x, float y, float w, float h, uint32_t argb);
void canvas_fill_round(Canvas *c, float x, float y, float w, float h, float radius, uint32_t argb);
// The stroke stays inside the box.
void canvas_stroke_round(Canvas *c, float x, float y, float w, float h, float radius, float width,
                         uint32_t argb);
void canvas_fill_circle(Canvas *c, float cx, float cy, float radius, uint32_t argb);
void canvas_stroke_circle(Canvas *c, float cx, float cy, float radius, float width, uint32_t argb);
void canvas_line(Canvas *c, float x0, float y0, float x1, float y1, float width, uint32_t argb);
// A filled triangle through three points (x, y pairs).
void canvas_fill_triangle(Canvas *c, const float points[6], uint32_t argb);

// stroke_round in 3 px dashes (for a 1 px line), like a CSS dashed border.
void canvas_stroke_round_dashed(Canvas *c, float x, float y, float w, float h, float radius, float width,
                                uint32_t argb);

// A box of argb that fades out vertically: solid over the solid_part (0..1)
// at the top (or at the bottom when !solid_top), transparent at the other
// end, like a CSS linear-gradient edge over a scrolled list.
void canvas_fill_fade(Canvas *c, float x, float y, float w, float h, uint32_t argb, bool solid_top,
                      float solid_part);

// Draws what follows scaled by (sx, sy), then moved by (dx, dy). (1, 1, 0,
// 0) draws normally again.
void canvas_set_transform(Canvas *c, float sx, float sy, float dx, float dy);
// Draws what follows at opacity (0..1) until the matching pop. Pushes do
// not nest.
void canvas_push_opacity(Canvas *c, float opacity);
void canvas_pop_opacity(Canvas *c);

// Clips drawing to a box until the matching pop.
void canvas_push_clip(Canvas *c, float x, float y, float w, float h);
void canvas_pop_clip(Canvas *c);
