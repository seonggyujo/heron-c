// Tests for keys, mouse reports and pasting.
#include <stdlib.h>
#include <windows.h>

#include "check.h"
#include "input/keys.h"
#include "input/mouse_report.h"
#include "input/paste.h"

static const char *encoded(int n, char *buf) {
  buf[n] = 0;
  return buf;
}

void test_keys(void) {
  char out[17];
  KeyMods none = {0}, ctrl = {.ctrl = true}, shift = {.shift = true};
  CHECK_STR(encoded(keys_encode(VK_UP, none, false, out), out), "\x1b[A");
  CHECK_STR(encoded(keys_encode(VK_UP, none, true, out), out), "\x1bOA");
  CHECK_STR(encoded(keys_encode(VK_LEFT, ctrl, true, out), out), "\x1b[1;5D");
  CHECK_STR(encoded(keys_encode(VK_DELETE, none, false, out), out), "\x1b[3~");
  CHECK_STR(encoded(keys_encode(VK_F5, shift, false, out), out), "\x1b[15;2~");
  CHECK_STR(encoded(keys_encode(VK_F1, none, false, out), out), "\x1bOP");
  CHECK_STR(encoded(keys_encode(VK_TAB, shift, false, out), out), "\x1b[Z");
  CHECK(keys_encode(VK_TAB, none, false, out) == 0);
  CHECK(keys_encode('A', ctrl, false, out) == 0);
  CHECK(keys_encode(VK_SPACE, ctrl, false, out) == 1 && out[0] == 0);
  // Esc and Ctrl+C stop an agent's turn; with other modifiers they do not.
  CHECK(keys_is_interrupt(0x1B, none, false) && keys_is_interrupt(0x03, ctrl, false));
  CHECK(!keys_is_interrupt(0x1B, ctrl, false) && !keys_is_interrupt(0x1B, shift, false));
  CHECK(!keys_is_interrupt(0x03, none, false) && !keys_is_interrupt(0x1B, none, true) && !keys_is_interrupt('c', ctrl, false));
  CHECK_STR(encoded(keys_encode_char(0x08, false, out), out), "\x7f");
  CHECK_STR(encoded(keys_encode_char(0x7F, false, out), out), "\x08");
  CHECK_STR(encoded(keys_encode_char('x', true, out), out), "\x1bx");
  CHECK_STR(encoded(keys_encode_char(0xD55C, false, out), out), "\xed\x95\x9c");  // 한
}

void test_mouse_reports(void) {
  char out[33];
  KeyMods none = {0};
  CHECK_STR(encoded(mouse_report(MOUSE_LEFT, MOUSE_PRESS, 4, 9, none, true, out), out), "\x1b[<0;5;10M");
  CHECK_STR(encoded(mouse_report(MOUSE_LEFT, MOUSE_RELEASE, 4, 9, none, true, out), out),
            "\x1b[<0;5;10m");
  CHECK_STR(encoded(mouse_report(MOUSE_WHEEL_UP, MOUSE_PRESS, 0, 0, none, true, out), out),
            "\x1b[<64;1;1M");
  CHECK_STR(encoded(mouse_report(MOUSE_LEFT, MOUSE_MOTION, 1, 1, none, true, out), out),
            "\x1b[<32;2;2M");
  CHECK_STR(encoded(mouse_report(MOUSE_LEFT, MOUSE_PRESS, 0, 0, none, false, out), out), "\x1b[M !!");
  CHECK(mouse_report(MOUSE_LEFT, MOUSE_PRESS, 300, 0, none, false, out) == 0);
  CHECK(mouse_wanted(1000, MOUSE_PRESS, false));
  CHECK(!mouse_wanted(1000, MOUSE_MOTION, true));
  CHECK(mouse_wanted(1002, MOUSE_MOTION, true) && !mouse_wanted(1002, MOUSE_MOTION, false));
  CHECK(mouse_wanted(1003, MOUSE_MOTION, false));
}

void test_paste(void) {
  size_t n;
  char *p = paste_prepare("a\r\nb\nc", false, &n);
  CHECK_STR(p, "a\rb\rc");
  CHECK(n == 5);
  free(p);
  p = paste_prepare("x\x1b[201~y", true, &n);
  CHECK_STR(p, "\x1b[200~x[201~y\x1b[201~");
  free(p);
}
