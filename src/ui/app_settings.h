// Applying a setting to the running app and remembering it: theme,
// language, terminal font size, animations, and Reset.
#pragma once

#include <stdbool.h>

#include "app/strings.h"
#include "app/theme.h"
#include "ui/app.h"

// Reads the saved settings at startup (before the first frame).
void app_load_settings(App *a);

void app_set_theme(App *a, ThemeKind kind);
void app_set_lang(App *a, Lang lang);
// Clamped to 10..24 px; refits every pane.
void app_set_font_size(App *a, int px);
void app_set_motion(App *a, bool on);
// Settings > Reset: back to the defaults (the Claude Code hooks stay).
void app_reset_settings(App *a);
