// The terminal font (Cascadia Mono, else Consolas) at one size, and the size
// of one terminal cell in whole pixels.
#pragma once

typedef struct MonoFont MonoFont;

// px is the font size in pixels, already scaled for the screen. Returns
// NULL when neither font is installed.
MonoFont *mono_font_new(float px);
void mono_font_free(MonoFont *f);

int mono_font_cell_w(const MonoFont *f);
int mono_font_cell_h(const MonoFont *f);
