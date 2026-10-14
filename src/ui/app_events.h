// Window events that change the app as a whole: size, DPI, activation,
// and the two pty messages (new output, shell exit).
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void app_on_size(App *a, WPARAM kind, int width, int height);
void app_on_dpi(App *a, UINT dpi, const RECT *suggested);
void app_on_activate(App *a, bool active);
void app_on_output(App *a, int pane_id);
void app_on_exit(App *a, int pane_id, DWORD code);
