// UTF-8 <-> UTF-16 conversion at the Win32 boundary, and UTF-8 encoding of
// one code point. Returned strings are malloc'd and NUL-terminated.
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

// len < 0 means NUL-terminated.
wchar_t *utf8_to_wide(const char *s, ptrdiff_t len);

// len < 0 means NUL-terminated. out_len (optional) gets the byte count
// without the NUL.
char *wide_to_utf8(const wchar_t *w, ptrdiff_t len, size_t *out_len);

// Writes 1 to 4 bytes and returns the count. Surrogates and values past
// U+10FFFF become U+FFFD.
int utf8_encode(uint32_t cp, char out[4]);
