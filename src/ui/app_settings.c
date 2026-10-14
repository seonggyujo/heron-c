// Settings applied to the running app.
#include "ui/app_settings.h"

#include <string.h>

#include "app/prefs.h"
#include "base/log.h"
#include "ui/frame.h"
#include "ui/sidebar.h"

#define FONT_MIN 10
#define FONT_MAX 24
#define FONT_DEFAULT 14

static int clamp_font(int px) { return px < FONT_MIN ? FONT_MIN : px > FONT_MAX ? FONT_MAX : px; }

static ThemeKind saved_theme(void) {
  char v[16];
  return prefs_get(PREF_THEME, v, sizeof v) && strcmp(v, "black") == 0 ? THEME_BLACK : THEME_GREEN;
}

void app_load_settings(App *a) {
  theme_set(saved_theme());
  a->font_px = clamp_font(prefs_get_int(PREF_FONT_SIZE, FONT_DEFAULT));
  a->motion = prefs_get_bool(PREF_MOTION, true);
  a->buddies.on = prefs_get_bool(PREF_BUDDIES, true);
  strings_lang();  // reads the saved language
}

void app_set_theme(App *a, ThemeKind kind) {
  theme_set(kind);
  prefs_set(PREF_THEME, kind == THEME_BLACK ? "black" : "green");
  frame_apply_theme(a->hwnd);
  app_redraw(a);
}

void app_set_lang(App *a, Lang lang) {
  if (lang == strings_lang()) return;
  strings_set_lang(lang);
  log_info("settings", "language %s", lang == LANG_KO ? "ko" : "en");
  app_redraw(a);
}

static void refit(App *a) {
  app_set_dpi(a, a->dpi);
  app_layout(a);
  app_redraw(a);
}

void app_set_font_size(App *a, int px) {
  px = clamp_font(px);
  if (px == a->font_px) return;
  a->font_px = px;
  prefs_set_int(PREF_FONT_SIZE, px);
  refit(a);
}

void app_set_motion(App *a, bool on) {
  a->motion = on;
  prefs_set_bool(PREF_MOTION, on);
  log_info("settings", "animations %s", on ? "on" : "off");
  app_redraw(a);
}

void app_set_buddies(App *a, bool on) {
  a->buddies.on = on;
  prefs_set_bool(PREF_BUDDIES, on);
  log_info("settings", "terminal herons %s", on ? "on" : "off");
  app_redraw(a);
}

void app_reset_settings(App *a) {
  prefs_reset();
  strings_follow_system();
  theme_set(THEME_GREEN);
  frame_apply_theme(a->hwnd);
  a->motion = true;
  a->buddies.on = true;
  a->shell = shell_default();
  sidebar_reset(a);
  if (a->font_px != FONT_DEFAULT) {
    a->font_px = FONT_DEFAULT;
    refit(a);
  }
  app_redraw(a);
}
