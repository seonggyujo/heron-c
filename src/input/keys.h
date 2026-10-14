// Turning key presses into the bytes a terminal program expects, following
// xterm. Pure functions: the window passes the key and the terminal modes.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool shift, ctrl, alt;
} KeyMods;

// Keys that do not type a character (arrows, Home/End, Insert/Delete,
// PageUp/Down, F1-F12) and the few combinations Windows types the wrong
// character for (Shift+Tab, Ctrl+Space). vk is a Win32 virtual-key code.
// Returns the byte count, or 0 to leave the key to the character message.
int keys_encode(unsigned vk, KeyMods mods, bool app_cursor, char out[16]);

// A typed character. Backspace types 0x08 on Windows but terminals expect
// DEL (and Ctrl+Backspace the reverse); Alt adds an ESC prefix.
int keys_encode_char(uint32_t ch, bool alt, char out[16]);

// A typed character that stops a coding agent's turn: Esc (0x1B) with no
// modifier, or Ctrl+C (0x03) without Alt or Shift (Heron's
// isInterruptKey). Both reach the window as characters, not as keys.
bool keys_is_interrupt(uint32_t ch, KeyMods mods, bool alt);
