// The Direct2D objects behind a Canvas, for the text drawing in render/.
// Nothing outside render/ includes this.
#pragma once

#include <d2d1.h>

#include "render/canvas.h"

ID2D1RenderTarget *canvas_target(Canvas *c);
ID2D1Factory *canvas_factory(Canvas *c);
// Shared drawing objects, made on first use. NULL when that failed.
ID2D1Layer *canvas_layer(Canvas *c);
ID2D1StrokeStyle *canvas_dash_style(Canvas *c);

D2D1_COLOR_F canvas_color(uint32_t argb);

// The canvas's one brush, set to argb. Valid until the next call.
ID2D1Brush *canvas_brush(Canvas *c, uint32_t argb);
