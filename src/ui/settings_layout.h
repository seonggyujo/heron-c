// Where the settings panel's rows and controls are. Only ui/settings*.c
// include this.
#pragma once

#include "ui/app.h"

typedef enum {
  SET_THEME_GREEN,
  SET_THEME_BLACK,
  SET_LANG_EN,
  SET_LANG_KO,
  SET_FONT_SMALLER,
  SET_FONT_VALUE,
  SET_FONT_LARGER,
  SET_MOTION,
  SET_CLOSE_ON_EXIT,
  SET_RESTORE,
  SET_HOOKS,
  SET_RESET,
  SET_COUNT,
} SettingsControl;

typedef enum {
  ROW_THEME,
  ROW_LANG,
  ROW_FONT,
  ROW_MOTION,
  ROW_CLOSE,
  ROW_RESTORE,
  ROW_HOOKS,
  ROW_RESET,
  ROW_COUNT
} SettingsRow;

typedef struct {
  Box panel;
  Box title;
  Box rows[ROW_COUNT];  // label side of each row
  Box controls[SET_COUNT];
  Box note;  // why the hooks could not be turned on or off, h 0 for none
  int separator_y;
} SettingsLayout;

// 300 px wide, right-aligned under the gear button.
void settings_layout(App *a, SettingsLayout *l);
// The control under (x, y), or -1.
int settings_control_at(const SettingsLayout *l, int x, int y);
