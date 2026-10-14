// Keys typed into the window: app shortcuts first (copy with a selection,
// paste, new pane, next pane), then the focused pane.
//   Ctrl+C          copy when text is selected, else ^C to the program
//   Ctrl+V          paste
//   Ctrl+Shift+T    new pane
//   Ctrl+Tab        next pane (Ctrl+Shift+Tab: previous)
#pragma once

#include "input/keys.h"
#include "ui/app.h"

KeyMods keyboard_mods(void);

// WM_KEYDOWN and WM_SYSKEYDOWN. Returns true when handled; the character
// message the same key also produced is then dropped.
bool keyboard_key_down(App *a, WPARAM vk);

// WM_CHAR, and WM_SYSCHAR with alt set.
void keyboard_char(App *a, WPARAM ch, bool alt);

// Copies the focused pane's selection. Returns false when nothing is selected.
bool keyboard_copy(App *a, Pane *p);
void keyboard_paste(App *a, Pane *p);
