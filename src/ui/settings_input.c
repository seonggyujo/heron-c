// What each settings control does when pressed.
#include "app/prefs.h"
#include "app/strings.h"
#include "base/log.h"
#include "ui/animation.h"
#include "ui/app_settings.h"
#include "ui/hooks_state.h"
#include "ui/settings_layout.h"
#include "ui/settings_panel.h"

#define CONFIRM_MS 3000
#define RESULT_MS 2000

Box settings_panel_box(App *a) {
  SettingsLayout l;
  settings_layout(a, &l);
  return l.panel;
}

const char *settings_tip_at(App *a, int x, int y, Box *box) {
  SettingsLayout l;
  settings_layout(a, &l);
  int control = settings_control_at(&l, x, y);
  const char *tips[SET_COUNT] = {[SET_THEME_GREEN] = strings()->theme_green,
                                 [SET_THEME_BLACK] = strings()->theme_black,
                                 [SET_BUDDIES_OFF] = strings()->buddies_hint,
                                 [SET_BUDDIES_1] = strings()->buddies_hint,
                                 [SET_BUDDIES_2] = strings()->buddies_hint,
                                 [SET_BUDDIES_3] = strings()->buddies_hint,
                                 [SET_HOOKS] = strings()->agent_hooks_hint,
                                 [SET_RESET] = strings()->reset_hint};
  if (control < 0 || !tips[control]) return NULL;
  *box = l.controls[control];
  return tips[control];
}

bool settings_mouse_move(App *a, int x, int y) {
  SettingsLayout l;
  settings_layout(a, &l);
  int hover = settings_control_at(&l, x, y);
  if (hover != a->popover_hover) {
    a->popover_hover = hover;
    app_redraw(a);
  }
  return box_contains(l.panel, x, y);
}

static void toggle(App *a, const char *key, const char *what) {
  bool on = !prefs_get_bool(key, true);
  prefs_set_bool(key, on);
  log_info("settings", "%s %s", what, on ? "on" : "off");
}

// The first press only asks; a second one within CONFIRM_MS resets.
static void press_reset(App *a) {
  if (a->reset_phase == RESET_CONFIRM) {
    app_reset_settings(a);
    a->reset_phase = RESET_DONE;
    SetTimer(a->hwnd, TIMER_RESET_BUTTON, RESULT_MS, NULL);
  } else {
    a->reset_phase = RESET_CONFIRM;
    SetTimer(a->hwnd, TIMER_RESET_BUTTON, CONFIRM_MS, NULL);
  }
}

static void press(App *a, SettingsControl c) {
  if (c == SET_MOTION || c == SET_CLOSE_ON_EXIT || c == SET_RESTORE || (c == SET_HOOKS && !a->agents.hooks_busy)) {
    a->flipped_switch = (int)c;  // its knob slides (settings_paint.c)
    a->flipped_at = GetTickCount64();
  }
  switch (c) {
    case SET_THEME_GREEN: app_set_theme(a, THEME_GREEN); break;
    case SET_THEME_BLACK: app_set_theme(a, THEME_BLACK); break;
    case SET_LANG_EN: app_set_lang(a, LANG_EN); break;
    case SET_LANG_KO: app_set_lang(a, LANG_KO); break;
    case SET_FONT_SMALLER: app_set_font_size(a, a->font_px - 1); break;
    case SET_FONT_LARGER: app_set_font_size(a, a->font_px + 1); break;
    case SET_MOTION: app_set_motion(a, !a->motion); break;
    case SET_BUDDIES_OFF:
    case SET_BUDDIES_1:
    case SET_BUDDIES_2:
    case SET_BUDDIES_3: app_set_buddies(a, (int)c - SET_BUDDIES_OFF); break;
    case SET_CLOSE_ON_EXIT: toggle(a, PREF_CLOSE_ON_EXIT, "close pane on exit 0"); break;
    case SET_RESTORE: toggle(a, PREF_RESTORE_WORKSPACE, "reopen last work"); break;
    case SET_HOOKS: hooks_set(a, !a->agents.hooks_on); break;
    case SET_RESET: press_reset(a); break;
    default: break;
  }
}

bool settings_mouse_down(App *a, int x, int y) {
  SettingsLayout l;
  settings_layout(a, &l);
  if (!box_contains(l.panel, x, y)) return false;
  int c = settings_control_at(&l, x, y);
  if (c >= 0) press(a, (SettingsControl)c);
  anim_after_paint(a);
  app_redraw(a);
  return true;
}

void settings_reset_timer(App *a) {
  KillTimer(a->hwnd, TIMER_RESET_BUTTON);
  a->reset_phase = RESET_IDLE;
  app_redraw(a);
}
