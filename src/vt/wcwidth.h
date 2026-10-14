// How many terminal columns a code point takes: 0 for combining marks and
// other zero-width characters, 2 for East Asian wide characters and emoji,
// 1 for everything else.
#pragma once

#include <stdint.h>

int vt_wcwidth(uint32_t cp);
