// Drawing the title bar's buttons. Only ui/titlebar*.c include this.
#pragma once

#include <stddef.h>

#include "ui/titlebar.h"

// "+  New terminal": the split button's label before the shell name.
void titlebar_label(App *a, char *out, size_t cap);
// Width of the shell name pill: fits the longest name, so switching shells
// never moves the title bar buttons.
int titlebar_pill_width(App *a);
// "2 running" or "1 / 2 running".
void titlebar_count_text(App *a, char *out, size_t cap);
void titlebar_paint_tidy(App *a, const TitleLayout *t);
void titlebar_paint_settings(App *a, const TitleLayout *t);
void titlebar_paint_split_button(App *a, const TitleLayout *t);
void titlebar_paint_lights(App *a, const TitleLayout *t);
