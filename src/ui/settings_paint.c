// Drawing the settings panel.
#include <stdio.h>

#include "app/prefs.h"
#include "app/strings.h"
#include "app/theme.h"
#include "base/easing.h"
#include "ui/settings_layout.h"
#include "ui/settings_panel.h"
#include "ui/widgets.h"

static void paint_title(App *a, Box b) {
  widget_spaced_text(a, a->brand_font, strings()->settings, (float)b.x, (float)b.y, (float)b.h, 1.5f * a->scale,
                     THEME_FG_DIM);
}

static const char *row_label(SettingsRow row) {
  const Strings *s = strings();
  const char *labels[ROW_COUNT] = {s->theme,         s->language,          s->font_size,   s->animations, s->buddies,
                                   s->close_on_exit, s->restore_workspace, s->agent_hooks, s->settings};
  return labels[row];
}

// Where a switch's knob is: the one just pressed slides over 120 ms
// (ease-out, as settings.css).
static float switch_at(const App *a, SettingsControl c, bool on) {
  ULONGLONG t = GetTickCount64() - a->flipped_at;
  if (!a->motion || a->flipped_switch != (int)c || t >= 120) return on ? 1.0f : 0.0f;
  float p = ease(EASE_OUT_CSS, t / 120.0f);
  return on ? p : 1 - p;
}

static void paint_controls(App *a, const SettingsLayout *l) {
  int hover = a->popover_hover;
  bool black = theme_kind() == THEME_BLACK;
  widget_swatch(a, l->controls[SET_THEME_GREEN], 0xFF8FD8A8, !black, hover == SET_THEME_GREEN);
  widget_swatch(a, l->controls[SET_THEME_BLACK], 0xFF1C1C1F, black, hover == SET_THEME_BLACK);

  const char *langs[2] = {"English", "한국어"};
  int lang_hover = hover == SET_LANG_EN ? 0 : hover == SET_LANG_KO ? 1 : -1;
  widget_group(a, &l->controls[SET_LANG_EN], 2, a->small_font, langs, strings_lang() == LANG_KO, lang_hover);

  char size[8];
  snprintf(size, sizeof size, "%d", a->font_px);
  const char *font[3] = {"A\xe2\x88\x92", NULL, "A+"};  // A−
  int font_hover = hover == SET_FONT_SMALLER ? 0 : hover == SET_FONT_LARGER ? 2 : -1;
  widget_group(a, &l->controls[SET_FONT_SMALLER], 3, a->header_font, font, -1, font_hover);
  Box v = l->controls[SET_FONT_VALUE];
  ui_text_draw(a->canvas, a->header_font, size, (float)v.x, (float)v.y, (float)v.w, (float)v.h, THEME_FG_DIM,
               ALIGN_CENTER);

  widget_switch(a, l->controls[SET_MOTION], switch_at(a, SET_MOTION, a->motion), hover == SET_MOTION);
  const char *herons[4] = {strings()->buddies_off, "1", "2", "3"};
  int herons_hover = hover >= SET_BUDDIES_OFF && hover <= SET_BUDDIES_3 ? hover - SET_BUDDIES_OFF : -1;
  widget_group(a, &l->controls[SET_BUDDIES_OFF], 4, a->small_font, herons, a->buddies.herons, herons_hover);
  bool close = prefs_get_bool(PREF_CLOSE_ON_EXIT, true), restore = prefs_get_bool(PREF_RESTORE_WORKSPACE, true);
  widget_switch(a, l->controls[SET_CLOSE_ON_EXIT], switch_at(a, SET_CLOSE_ON_EXIT, close), hover == SET_CLOSE_ON_EXIT);
  widget_switch(a, l->controls[SET_RESTORE], switch_at(a, SET_RESTORE, restore), hover == SET_RESTORE);
  // While it changes, the switch shows where it goes.
  const AgentsState *agents = &a->agents;
  bool hooks = agents->hooks_busy ? !agents->hooks_on : agents->hooks_on;
  widget_switch(a, l->controls[SET_HOOKS], switch_at(a, SET_HOOKS, hooks), hover == SET_HOOKS && !agents->hooks_busy);
  if (l->note.h) {
    ui_text_wrap(a->canvas, a->small_font, agents->hooks_error, (float)l->note.x, (float)l->note.y,
                 (float)l->note.w, 11 * 1.4f * a->scale, THEME_AGENT_BAD);
  }

  Box reset = l->controls[SET_RESET];
  if (a->reset_phase == RESET_CONFIRM) {
    widget_danger_button(a, reset, a->ui_font, strings()->sure);
  } else {
    const char *text = a->reset_phase == RESET_DONE ? strings()->done : strings()->reset;
    widget_button(a, reset, a->ui_font, text, hover == SET_RESET ? WIDGET_HOVER : WIDGET_NORMAL);
  }
}

void settings_paint(App *a) {
  SettingsLayout l;
  settings_layout(a, &l);
  widget_panel(a, l.panel, 10 * a->scale);
  paint_title(a, l.title);
  for (int r = 0; r < ROW_COUNT; r++) {
    Box b = l.rows[r];
    ui_text_draw(a->canvas, a->ui_font, row_label((SettingsRow)r), (float)b.x, (float)b.y, b.w * 0.6f,
                 (float)b.h, THEME_FG_SOFT, ALIGN_LEFT);
  }
  canvas_fill_rect(a->canvas, (float)l.title.x, (float)l.separator_y, (float)l.title.w, 1, THEME_BORDER_SOFT);
  paint_controls(a, &l);
}
