// Key presses to terminal bytes, xterm style.
#include "input/keys.h"

#include <stdio.h>
#include <windows.h>

#include "base/utf.h"

// xterm modifier parameter: 1 + Shift + 2 Alt + 4 Ctrl.
static int modifier_param(KeyMods m) { return 1 + (m.shift ? 1 : 0) + (m.alt ? 2 : 0) + (m.ctrl ? 4 : 0); }

// Arrows, Home, End and F1-F4: ESC [ 1 ; m X with modifiers, otherwise
// ESC O X in application mode (always for F1-F4) or ESC [ X.
static int letter_key(char final, KeyMods m, bool ss3, char *out) {
  int mod = modifier_param(m);
  if (mod > 1) return sprintf(out, "\x1b[1;%d%c", mod, final);
  return sprintf(out, ss3 ? "\x1bO%c" : "\x1b[%c", final);
}

// Insert, Delete, PageUp/Down, F5-F12: ESC [ n ~ or ESC [ n ; m ~.
static int tilde_key(int n, KeyMods m, char *out) {
  int mod = modifier_param(m);
  if (mod > 1) return sprintf(out, "\x1b[%d;%d~", n, mod);
  return sprintf(out, "\x1b[%d~", n);
}

int keys_encode(unsigned vk, KeyMods m, bool app_cursor, char out[16]) {
  switch (vk) {
    case VK_UP: return letter_key('A', m, app_cursor, out);
    case VK_DOWN: return letter_key('B', m, app_cursor, out);
    case VK_RIGHT: return letter_key('C', m, app_cursor, out);
    case VK_LEFT: return letter_key('D', m, app_cursor, out);
    case VK_HOME: return letter_key('H', m, app_cursor, out);
    case VK_END: return letter_key('F', m, app_cursor, out);
    case VK_F1: return letter_key('P', m, true, out);
    case VK_F2: return letter_key('Q', m, true, out);
    case VK_F3: return letter_key('R', m, true, out);
    case VK_F4: return letter_key('S', m, true, out);
    case VK_INSERT: return tilde_key(2, m, out);
    case VK_DELETE: return tilde_key(3, m, out);
    case VK_PRIOR: return tilde_key(5, m, out);
    case VK_NEXT: return tilde_key(6, m, out);
    case VK_F5: return tilde_key(15, m, out);
    case VK_F6: return tilde_key(17, m, out);
    case VK_F7: return tilde_key(18, m, out);
    case VK_F8: return tilde_key(19, m, out);
    case VK_F9: return tilde_key(20, m, out);
    case VK_F10: return tilde_key(21, m, out);
    case VK_F11: return tilde_key(23, m, out);
    case VK_F12: return tilde_key(24, m, out);
    case VK_TAB:
      if (m.shift && !m.ctrl) return sprintf(out, "\x1b[Z");
      return 0;
    case VK_SPACE:
      if (!m.ctrl) return 0;
      if (m.alt) {
        out[0] = 0x1b;
        out[1] = 0;
        return 2;
      }
      out[0] = 0;
      return 1;
    default: return 0;
  }
}

bool keys_is_interrupt(uint32_t ch, KeyMods mods, bool alt) {
  if (alt || mods.alt || mods.shift) return false;
  return mods.ctrl ? ch == 0x03 : ch == 0x1B;
}

int keys_encode_char(uint32_t ch, bool alt, char out[16]) {
  int n = 0;
  if (alt) out[n++] = 0x1b;
  if (ch == 0x08) {
    ch = 0x7F;
  } else if (ch == 0x7F) {
    ch = 0x08;
  }
  return n + utf8_encode(ch, out + n);
}
