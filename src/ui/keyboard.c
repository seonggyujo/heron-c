// Shortcuts, then keys and characters for the focused pane.
#include "ui/keyboard.h"

#include <stdlib.h>

#include "pane/pane_input.h"
#include "ui/agents.h"
#include "ui/clipboard.h"
#include "ui/header_drag.h"
#include "ui/popover.h"
#include "ui/sidebar.h"

KeyMods keyboard_mods(void) {
  KeyMods m = {GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_MENU) < 0};
  return m;
}

// TranslateMessage has already queued the character this key makes; drop it
// so the key is not sent twice.
static void drop_char(HWND hwnd) {
  MSG m;
  PeekMessageW(&m, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE);
  PeekMessageW(&m, hwnd, WM_SYSCHAR, WM_SYSCHAR, PM_REMOVE);
}

bool keyboard_copy(App *a, Pane *p) {
  if (!pane_has_selection(p)) return false;
  char *text = pane_take_selection(p);
  if (text) clipboard_set_text(a->hwnd, text);
  free(text);
  app_redraw(a);
  return true;
}

void keyboard_paste(App *a, Pane *p) {
  char *text = clipboard_get_text(a->hwnd);
  if (text) pane_paste(p, text);
  free(text);
  app_redraw(a);
}

static bool shortcut(App *a, WPARAM vk, KeyMods m) {
  if (!m.ctrl || m.alt) return false;
  Pane *p = panes_focused(&a->panes);
  if (m.shift && vk == 'T') {
    app_open_pane(a);
    return true;
  }
  if (vk == VK_TAB) {
    app_focus_step(a, m.shift ? -1 : 1);
    return true;
  }
  if (p && vk == 'C') return keyboard_copy(a, p);
  if (p && vk == 'V') {
    keyboard_paste(a, p);
    return true;
  }
  return false;
}

bool keyboard_key_down(App *a, WPARAM vk) {
  if (vk == VK_PROCESSKEY) return false;  // the IME is using this key
  KeyMods m = keyboard_mods();
  bool done = header_drag_key(a, vk) || popover_key(a, vk) || sidebar_key(a, vk) || shortcut(a, vk, m);
  Pane *p = panes_focused(&a->panes);
  if (!done && p && pane_key(p, (unsigned)vk, m)) {
    agents_key(a, p, false);
    done = true;
  }
  if (!done) return false;
  drop_char(a->hwnd);
  app_redraw(a);
  return true;
}

void keyboard_char(App *a, WPARAM ch, bool alt) {
  uint32_t cp = (uint32_t)ch;
  if (cp >= 0xD800 && cp <= 0xDBFF) {
    a->high_surrogate = (WCHAR)cp;
    return;
  }
  if (cp >= 0xDC00 && cp <= 0xDFFF) {
    if (!a->high_surrogate) return;
    cp = 0x10000 + ((uint32_t)(a->high_surrogate - 0xD800) << 10) + (cp - 0xDC00);
  }
  a->high_surrogate = 0;
  Pane *p = panes_focused(&a->panes);
  if (!p) return;
  pane_char(p, cp, alt);
  agents_key(a, p, keys_is_interrupt(cp, keyboard_mods(), alt));
  app_redraw(a);
}
