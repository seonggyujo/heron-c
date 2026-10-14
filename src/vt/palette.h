// The terminal's default colors, the same as xterm.js out of the box: white
// on black, the Tango 16 ANSI colors, the 6x6x6 cube and 24 grays.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PALETTE_FG 0xFFFFFFu
#define PALETTE_BG 0x000000u
#define PALETTE_CURSOR 0xFFFFFFu

// Entry 0 to 255 as 0xRRGGBB.
uint32_t palette_rgb(int index);

// A cell color as 0xRRGGBB. COLOR_DEFAULT is the terminal's foreground or
// background depending on which one is asked for.
uint32_t color_to_rgb(uint32_t color, bool foreground);
