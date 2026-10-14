// Settings panel geometry (Heron's settings.css: 300 px wide, 12/14 px
// padding, 30 px rows 6 px apart, the hooks error in 11 px lines at most
// 240 px wide under its row, a separator before Reset).
#include "ui/settings_layout.h"

#include "app/strings.h"
#include "ui/titlebar.h"

// Lays out n boxes of the given widths right-aligned at right, vertically
// centered in a row starting at y.
static void right_aligned(Box *out, const int *widths, int n, int right, int y, int row_h, int h, int gap) {
  int x = right;
  for (int i = n - 1; i >= 0; i--) {
    x -= widths[i];
    out[i] = (Box){x, y + (row_h - h) / 2, widths[i], h};
    x -= gap;
  }
}

static void place_row(App *a, SettingsLayout *l, SettingsRow row, int y, int left, int right) {
  int row_h = app_px(a, 30);
  l->rows[row] = (Box){left, y, right - left, row_h};
  switch (row) {
    case ROW_THEME: {
      int w[2] = {app_px(a, 16), app_px(a, 16)};
      right_aligned(&l->controls[SET_THEME_GREEN], w, 2, right - app_px(a, 4), y, row_h, app_px(a, 16), app_px(a, 6));
      break;
    }
    case ROW_LANG: {
      int w[2] = {(int)ui_text_width(a->small_font, "English") + app_px(a, 18),
                  (int)ui_text_width(a->small_font, "한국어") + app_px(a, 18)};
      right_aligned(&l->controls[SET_LANG_EN], w, 2, right, y, row_h, app_px(a, 24), 0);
      break;
    }
    case ROW_FONT: {
      int w[3] = {app_px(a, 30), app_px(a, 30), app_px(a, 30)};
      right_aligned(&l->controls[SET_FONT_SMALLER], w, 3, right, y, row_h, app_px(a, 26), 0);
      break;
    }
    case ROW_RESET: {
      int w = (int)ui_text_width(a->ui_font, strings()->reset) + app_px(a, 24);
      if (w < app_px(a, 96)) w = app_px(a, 96);
      right_aligned(&l->controls[SET_RESET], &w, 1, right, y, row_h, app_px(a, 26), 0);
      break;
    }
    default: {  // a switch
      static const SettingsControl SWITCHES[] = {[ROW_MOTION] = SET_MOTION, [ROW_CLOSE] = SET_CLOSE_ON_EXIT,
                                                 [ROW_RESTORE] = SET_RESTORE, [ROW_HOOKS] = SET_HOOKS};
      int w = app_px(a, 34);
      right_aligned(&l->controls[SWITCHES[row]], &w, 1, right, y, row_h, app_px(a, 18), 0);
    }
  }
}

// The hooks error under its row; returns the room it takes.
static int place_note(App *a, SettingsLayout *l, int y, int left, int right) {
  l->note = (Box){0};
  const char *error = a->agents.hooks_error;
  if (!error[0]) return 0;
  int w = right - left < app_px(a, 240) ? right - left : app_px(a, 240);
  float line = 11 * 1.4f * a->scale;
  int lines = ui_text_wrap(NULL, a->small_font, error, 0, 0, (float)w, line, 0);
  l->note = (Box){left, y, w, (int)(lines * line + 0.5f)};
  return l->note.h + app_px(a, 6);
}

void settings_layout(App *a, SettingsLayout *l) {
  TitleLayout t;
  titlebar_layout(a, &t);
  int w = app_px(a, 300), right = t.settings.x + t.settings.w;
  int x = right - w, y = t.settings.y + t.settings.h + app_px(a, 6);
  int left = x + app_px(a, 14), inner_right = x + w - app_px(a, 14);
  int cursor = y + app_px(a, 12);
  l->title = (Box){left, cursor, inner_right - left, app_px(a, 16)};
  cursor += app_px(a, 16 + 4 + 6);
  for (int r = 0; r < ROW_COUNT; r++) {
    if (r == ROW_RESET) {
      l->separator_y = cursor + app_px(a, 4) - app_px(a, 6) / 2;
      cursor += app_px(a, 1 + 8);
    }
    place_row(a, l, (SettingsRow)r, cursor, left, inner_right);
    cursor += app_px(a, 30 + 6);
    if (r == ROW_HOOKS) cursor += place_note(a, l, cursor, left, inner_right);
  }
  l->panel = (Box){x, y, w, cursor - app_px(a, 6) + app_px(a, 14) - y};
}

int settings_control_at(const SettingsLayout *l, int x, int y) {
  for (int i = 0; i < SET_COUNT; i++) {
    if (i != SET_FONT_VALUE && box_contains(l->controls[i], x, y)) return i;
  }
  return -1;
}
