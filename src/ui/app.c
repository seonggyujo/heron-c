// Making the app state, sizing it for the DPI, and fitting panes to the window.
#include "ui/app.h"

#include <math.h>
#include <stdlib.h>

#include "app/prefs.h"
#include "base/log.h"
#include "base/mem.h"
#include "ui/app_settings.h"
#include "ui/sidebar.h"
#include "ui/usage_bar.h"

static int px(float scale, float v) { return (int)lroundf(v * scale); }

static void set_metrics(App *a) {
  float k = a->scale;
  a->m.titlebar_h = px(k, 44);
  a->m.gap = px(k, 10);
  a->m.resize_border = px(k, 8);
  PaneMetrics *p = &a->m.pane;
  p->header_h = px(k, 28);
  p->pad_left = px(k, 9);  // 1px border and 8px padding, as in Heron
  p->pad_top = px(k, 4);
  p->pad_right = px(k, 5);
  p->pad_bottom = px(k, 5);
  p->radius = px(k, 10);
  p->close_size = px(k, 22);
  p->close_right = px(k, 6);
}

static void free_fonts(App *a) {
  mono_font_free(a->font);
  ui_font_free(a->ui_font);
  ui_font_free(a->small_font);
  ui_font_free(a->huge_font);
  ui_font_free(a->header_font);
  ui_font_free(a->brand_font);
  ui_font_free(a->big_font);
  for (int i = 0; i < FONT_COUNT; i++) ui_font_free(a->fonts[i]);
}

static const struct {
  bool mono;
  float px;
  int weight;
} STYLES[FONT_COUNT] = {
    [FONT_UI_12] = {false, 12, 400},      [FONT_UI_12_SEMI] = {false, 12, 600},
    [FONT_UI_13] = {false, 13, 400},      [FONT_UI_14] = {false, 14, 400},
    [FONT_UI_14_SEMI] = {false, 14, 600}, [FONT_UI_15_SEMI] = {false, 15, 600},
    [FONT_UI_16] = {false, 16, 400},      [FONT_MONO_11] = {true, 11, 400},
    [FONT_MONO_13_BOLD] = {true, 13, 700}, [FONT_MONO_22_BOLD] = {true, 22, 700},
};

static void make_fonts(App *a) {
  for (int i = 0; i < FONT_COUNT; i++) {
    const wchar_t *family = STYLES[i].mono ? L"Cascadia Mono" : L"Segoe UI Variable Text";
    a->fonts[i] = ui_font_new(family, STYLES[i].px * a->scale, STYLES[i].weight);
  }
}

bool app_set_dpi(App *a, UINT dpi) {
  a->dpi = dpi;
  a->scale = dpi / 96.0f;
  set_metrics(a);
  free_fonts(a);
  float k = a->scale;
  a->font = mono_font_new(a->font_px * k);
  a->ui_font = ui_font_new(L"Segoe UI Variable Text", 12.5f * k, 400);
  a->small_font = ui_font_new(L"Segoe UI Variable Text", 11 * k, 400);
  a->header_font = ui_font_new(L"Cascadia Mono", 12 * k, 400);
  a->brand_font = ui_font_new(L"Cascadia Mono", 12 * k, 600);
  a->big_font = ui_font_new(L"Cascadia Mono", 13 * k, 400);
  a->huge_font = ui_font_new(L"Cascadia Mono", 48 * k, 400);
  make_fonts(a);
  log_debug("app", "dpi %u, scale %.2f", dpi, k);
  return a->font != NULL;
}

App *app_new(HWND hwnd, const wchar_t *start_dir) {
  App *a = xcalloc(1, sizeof *a);
  a->hwnd = hwnd;
  a->canvas = canvas_new(hwnd);
  a->start_dir = start_dir ? _wcsdup(start_dir) : NULL;
  a->divider_hover = -1;
  app_load_settings(a);
  sidebar_load(a);
  char id[32];
  ShellKind kind;
  bool saved = prefs_get(PREF_SHELL, id, sizeof id) && shell_from_id(id, &kind) && shell_installed(kind);
  a->shell = saved ? kind : shell_default();
  return a;
}

void app_free(App *a) {
  if (!a) return;
  free_fonts(a);
  canvas_free(a->canvas);
  free(a->start_dir);
  free(a->saved_work);
  free(a);
}

Box app_workspace(const App *a) {
  RECT rc;
  GetClientRect(a->hwnd, &rc);
  int left = sidebar_target_width(a), top = a->m.titlebar_h, bottom = rc.bottom - usage_bar_height(a);
  Box b = {left, top, rc.right - left, bottom - top};
  return b;
}

PaneLayout app_pane_layout(const App *a) {
  PaneLayout l = {app_workspace(a), a->m.gap, &a->m.pane, a->font};
  return l;
}

void app_layout(App *a) {
  PaneLayout l = app_pane_layout(a);
  panes_layout(&a->panes, &l);
}

int app_px(const App *a, float css_px) { return px(a->scale, css_px); }

void app_redraw(App *a) { InvalidateRect(a->hwnd, NULL, FALSE); }
