// Keyboard and clipboard input for a pane. Anything typed or pasted snaps the
// view back to the live screen and clears the selection, as xterm.js does.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "input/keys.h"
#include "pane/pane.h"

void pane_send(Pane *p, const char *data, size_t len);

// A key that types no character (see keys_encode). Returns true when it
// was sent, false to leave it to the character message.
bool pane_key(Pane *p, unsigned vk, KeyMods mods);
void pane_char(Pane *p, uint32_t ch, bool alt);
void pane_paste(Pane *p, const char *utf8);

// The selected text (caller frees), and the selection is cleared. NULL when
// nothing is selected.
char *pane_take_selection(Pane *p);
bool pane_has_selection(const Pane *p);

// Tells a program that asked for focus reports (mode 1004).
void pane_focus_changed(Pane *p, bool focused);
