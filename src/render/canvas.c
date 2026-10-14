// A Direct2D window render target and the shapes drawn on it.
#include <initguid.h>

#include "render/canvas_d2d.h"

#include <stdlib.h>

#include "base/log.h"
#include "base/mem.h"
#include "render/fonts.h"

struct Canvas {
  HWND hwnd;
  ID2D1Factory *factory;
  ID2D1HwndRenderTarget *hwnd_target;
  ID2D1RenderTarget *target;  // the same object, as its base interface
  ID2D1SolidColorBrush *brush;
  ID2D1Layer *layer;       // for canvas_push_opacity, made on first use
  ID2D1StrokeStyle *dash;  // for dashed strokes, made on first use
};

D2D1_COLOR_F canvas_color(uint32_t argb) {
  D2D1_COLOR_F c = {((argb >> 16) & 0xFF) / 255.0f, ((argb >> 8) & 0xFF) / 255.0f,
                    (argb & 0xFF) / 255.0f, ((argb >> 24) & 0xFF) / 255.0f};
  return c;
}

static void release_target(Canvas *c) {
  if (c->brush) ID2D1SolidColorBrush_Release(c->brush);
  if (c->layer) ID2D1Layer_Release(c->layer);
  if (c->hwnd_target) ID2D1HwndRenderTarget_Release(c->hwnd_target);
  c->brush = NULL;
  c->layer = NULL;
  c->hwnd_target = NULL;
  c->target = NULL;
}

static bool create_target(Canvas *c) {
  RECT rc;
  GetClientRect(c->hwnd, &rc);
  D2D1_RENDER_TARGET_PROPERTIES props = {
      D2D1_RENDER_TARGET_TYPE_DEFAULT, {DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN}, 96.0f,
      96.0f, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT};
  D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_props = {
      c->hwnd, {(UINT32)rc.right, (UINT32)rc.bottom}, D2D1_PRESENT_OPTIONS_NONE};
  HRESULT hr = ID2D1Factory_CreateHwndRenderTarget(c->factory, &props, &hwnd_props, &c->hwnd_target);
  if (FAILED(hr)) {
    log_error("canvas", "CreateHwndRenderTarget failed: 0x%08lx", (unsigned long)hr);
    return false;
  }
  c->target = (ID2D1RenderTarget *)c->hwnd_target;
  D2D1_COLOR_F white = {1, 1, 1, 1};
  ID2D1RenderTarget_CreateSolidColorBrush(c->target, &white, NULL, &c->brush);
  ID2D1RenderTarget_SetTextAntialiasMode(c->target, D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
  // Without contrast enhancement light text on black keeps the weight the
  // font was drawn with instead of looking a step bolder.
  IDWriteRenderingParams *params = NULL;
  if (fonts_factory() &&
      SUCCEEDED(IDWriteFactory_CreateCustomRenderingParams(fonts_factory(), 1.8f, 0.0f, 0.0f,
                                                           DWRITE_PIXEL_GEOMETRY_FLAT,
                                                           DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC, &params))) {
    ID2D1RenderTarget_SetTextRenderingParams(c->target, params);
    IDWriteRenderingParams_Release(params);
  }
  return true;
}

Canvas *canvas_new(HWND hwnd) {
  Canvas *c = xcalloc(1, sizeof *c);
  c->hwnd = hwnd;
  D2D1_FACTORY_OPTIONS options = {D2D1_DEBUG_LEVEL_NONE};
  HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, &options,
                                 (void **)&c->factory);
  if (FAILED(hr)) log_error("canvas", "D2D1CreateFactory failed: 0x%08lx", (unsigned long)hr);
  return c;
}

void canvas_free(Canvas *c) {
  if (!c) return;
  release_target(c);
  if (c->dash) ID2D1StrokeStyle_Release(c->dash);
  if (c->factory) ID2D1Factory_Release(c->factory);
  free(c);
}

void canvas_resize(Canvas *c, int width, int height) {
  if (!c->hwnd_target) return;
  D2D1_SIZE_U size = {(UINT32)width, (UINT32)height};
  ID2D1HwndRenderTarget_Resize(c->hwnd_target, &size);
}

bool canvas_begin(Canvas *c) {
  if (!c->factory) return false;
  if (!c->target && !create_target(c)) return false;
  ID2D1RenderTarget_BeginDraw(c->target);
  return true;
}

void canvas_end(Canvas *c) {
  HRESULT hr = ID2D1RenderTarget_EndDraw(c->target, NULL, NULL);
  // The GPU was reset or changed: start over with a new target next frame.
  if (hr == (HRESULT)D2DERR_RECREATE_TARGET) {
    log_warn("canvas", "render target lost, recreating");
    release_target(c);
    InvalidateRect(c->hwnd, NULL, FALSE);
  }
}

ID2D1RenderTarget *canvas_target(Canvas *c) { return c->target; }

ID2D1Factory *canvas_factory(Canvas *c) { return c->factory; }

ID2D1Layer *canvas_layer(Canvas *c) {
  if (!c->layer) ID2D1RenderTarget_CreateLayer(c->target, NULL, &c->layer);
  return c->layer;
}

ID2D1StrokeStyle *canvas_dash_style(Canvas *c) {
  if (!c->dash) {
    D2D1_STROKE_STYLE_PROPERTIES props = {D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT,
                                          D2D1_LINE_JOIN_MITER, 10.0f, D2D1_DASH_STYLE_CUSTOM, 0.0f};
    const FLOAT dashes[2] = {3, 3};  // in stroke widths, as a CSS dashed border
    ID2D1Factory_CreateStrokeStyle(c->factory, &props, dashes, 2, &c->dash);
  }
  return c->dash;
}

ID2D1Brush *canvas_brush(Canvas *c, uint32_t argb) {
  D2D1_COLOR_F color = canvas_color(argb);
  ID2D1SolidColorBrush_SetColor(c->brush, &color);
  return (ID2D1Brush *)c->brush;
}
