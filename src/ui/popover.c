// Routing to the open dropdown.
#include "ui/popover.h"

#include "base/easing.h"
#include "ui/animation.h"
#include "ui/folder_menu.h"
#include "ui/settings_panel.h"
#include "ui/shell_menu.h"
#include "ui/sidebar.h"
#include "ui/titlebar.h"

#define FADE_MS 120

void popover_toggle(App *a, PopoverKind kind) {
  bool open = a->popover != kind;
  popover_close(a);  // the open one, if any, fades out
  if (!open) return;
  a->popover = kind;
  a->popover_opened_at = GetTickCount64();
  a->popover_hover = -1;
  if (kind == POPOVER_AGENT_FOLDERS) folder_menu_open(a);
  anim_after_paint(a);
  app_redraw(a);
}

void popover_close(App *a) {
  if (a->popover == POPOVER_NONE) return;
  a->popover_closing = a->popover;
  a->popover_closed_at = GetTickCount64();
  a->popover = POPOVER_NONE;
  anim_after_paint(a);
  app_redraw(a);
}

static bool fading(const App *a, ULONGLONG since, ULONGLONG now) { return a->motion && now - since < FADE_MS; }

bool popover_busy(const App *a, ULONGLONG now) {
  return (a->popover != POPOVER_NONE && fading(a, a->popover_opened_at, now)) ||
         (a->popover_closing != POPOVER_NONE && fading(a, a->popover_closed_at, now)) ||
         (a->popover == POPOVER_SETTINGS && fading(a, a->flipped_at, now));
}

static Box panel_of(App *a, PopoverKind kind) {
  if (kind == POPOVER_SHELLS) return shell_menu_panel(a);
  return kind == POPOVER_SETTINGS ? settings_panel_box(a) : folder_menu_panel(a);
}

static void paint_kind(App *a, PopoverKind kind) {
  if (kind == POPOVER_SHELLS) shell_menu_paint(a);
  if (kind == POPOVER_SETTINGS) settings_paint(a);
  if (kind == POPOVER_AGENT_FOLDERS) folder_menu_paint(a);
}

// shown: 0 hidden .. 1 in place.
static void paint_shown(App *a, PopoverKind kind, float shown) {
  if (shown >= 1) {
    paint_kind(a, kind);
    return;
  }
  Box p = panel_of(a, kind);
  float s = 0.98f + 0.02f * shown, ox = (float)(kind == POPOVER_AGENT_FOLDERS ? p.x : p.x + p.w);
  canvas_set_transform(a->canvas, s, s, ox * (1 - s), p.y * (1 - s) - 4 * a->scale * (1 - shown));
  canvas_push_opacity(a->canvas, shown);
  paint_kind(a, kind);
  canvas_pop_opacity(a->canvas);
  canvas_set_transform(a->canvas, 1, 1, 0, 0);
}

static float progress(const App *a, ULONGLONG since, ULONGLONG now) {
  return fading(a, since, now) ? ease(EASE_OUT_CSS, (float)(now - since) / FADE_MS) : 1;
}

void popover_paint(App *a) {
  ULONGLONG now = GetTickCount64();
  if (a->popover_closing != POPOVER_NONE) {
    float gone = progress(a, a->popover_closed_at, now);
    if (gone >= 1) a->popover_closing = POPOVER_NONE;
    else paint_shown(a, a->popover_closing, 1 - gone);
  }
  if (a->popover != POPOVER_NONE) paint_shown(a, a->popover, progress(a, a->popover_opened_at, now));
}

bool popover_mouse_move(App *a, int x, int y) {
  if (a->popover == POPOVER_SHELLS) return shell_menu_mouse_move(a, x, y);
  if (a->popover == POPOVER_SETTINGS) return settings_mouse_move(a, x, y);
  if (a->popover == POPOVER_AGENT_FOLDERS) return folder_menu_mouse_move(a, x, y);
  return false;
}

bool popover_contains(App *a, int x, int y) {
  if (a->popover == POPOVER_SHELLS) return box_contains(shell_menu_panel(a), x, y);
  if (a->popover == POPOVER_SETTINGS) return box_contains(settings_panel_box(a), x, y);
  if (a->popover == POPOVER_AGENT_FOLDERS) return box_contains(folder_menu_panel(a), x, y);
  return false;
}

// On the button that opened the open panel.
static bool on_anchor(App *a, int x, int y) {
  if (a->popover == POPOVER_AGENT_FOLDERS) return sidebar_menu_button_at(a, x, y);
  return titlebar_hit(a, x, y) == (a->popover == POPOVER_SHELLS ? HIT_SHELL_MENU : HIT_SETTINGS);
}

static bool press_inside(App *a, int x, int y) {
  switch (a->popover) {
    case POPOVER_SHELLS: return shell_menu_mouse_down(a, x, y);
    case POPOVER_SETTINGS: return settings_mouse_down(a, x, y);
    default: return folder_menu_mouse_down(a, x, y);
  }
}

bool popover_mouse_down(App *a, int x, int y) {
  if (a->popover == POPOVER_NONE) return false;
  if (press_inside(a, x, y)) return true;
  bool anchor = on_anchor(a, x, y);
  popover_close(a);
  return anchor;
}

bool popover_key(App *a, WPARAM vk) {
  if (a->popover == POPOVER_NONE) return false;
  if (vk == VK_ESCAPE) {
    popover_close(a);
    return true;
  }
  return a->popover == POPOVER_SHELLS && shell_menu_key(a, vk);
}
