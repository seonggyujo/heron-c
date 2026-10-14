// The DEC Special Graphics character set (ESC ( 0), which old programs use to
// draw boxes with plain ASCII letters.
#pragma once

#include <stdint.h>

// Maps an ASCII code point to its line-drawing character; others pass through.
uint32_t charset_dec_graphics(uint32_t cp);
