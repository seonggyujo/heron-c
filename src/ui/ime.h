// Korean (and other) IME input. The text being composed is drawn by the app
// at the cursor of the focused pane instead of in the IME's own window;
// finished text is sent to the pane. The candidate list opens at the cursor.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

// Handles the WM_IME_* messages. Returns true and sets *result when it did.
bool ime_message(App *a, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result);
