// UTF-8 <-> UTF-16 conversion through the Win32 API, and UTF-8 encoding.
#include "base/utf.h"

#include <windows.h>

#include "base/mem.h"

wchar_t *utf8_to_wide(const char *s, ptrdiff_t len) {
  int in_len = len < 0 ? -1 : (int)len;
  int n = in_len == 0 ? 0 : MultiByteToWideChar(CP_UTF8, 0, s, in_len, NULL, 0);
  // With -1 the count already includes the NUL; otherwise add room for it.
  size_t cap = (size_t)n + (len < 0 ? 0 : 1);
  if (cap == 0) cap = 1;
  wchar_t *w = xmalloc(cap * sizeof *w);
  if (n > 0) MultiByteToWideChar(CP_UTF8, 0, s, in_len, w, n);
  w[cap - 1] = 0;
  return w;
}

char *wide_to_utf8(const wchar_t *w, ptrdiff_t len, size_t *out_len) {
  int in_len = len < 0 ? -1 : (int)len;
  int n = in_len == 0 ? 0 : WideCharToMultiByte(CP_UTF8, 0, w, in_len, NULL, 0, NULL, NULL);
  size_t cap = (size_t)n + (len < 0 ? 0 : 1);
  if (cap == 0) cap = 1;
  char *s = xmalloc(cap);
  if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w, in_len, s, n, NULL, NULL);
  s[cap - 1] = 0;
  if (out_len) *out_len = cap - 1;
  return s;
}

int utf8_encode(uint32_t cp, char out[4]) {
  if ((cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF) cp = 0xFFFD;
  if (cp < 0x80) {
    out[0] = (char)cp;
    return 1;
  }
  if (cp < 0x800) {
    out[0] = (char)(0xC0 | (cp >> 6));
    out[1] = (char)(0x80 | (cp & 0x3F));
    return 2;
  }
  if (cp < 0x10000) {
    out[0] = (char)(0xE0 | (cp >> 12));
    out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
    out[2] = (char)(0x80 | (cp & 0x3F));
    return 3;
  }
  out[0] = (char)(0xF0 | (cp >> 18));
  out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
  out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
  out[3] = (char)(0x80 | (cp & 0x3F));
  return 4;
}
