// Small controls drawn in the frame theme, shared by the title bar, the
// dropdowns and the sidebar: buttons, joined button groups, switches, theme
// swatches, the dropdown panel and letter-spaced labels. Drawing only; each
// caller hit-tests the same boxes it drew.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

typedef enum { WIDGET_NORMAL, WIDGET_HOVER, WIDGET_PRESSED, WIDGET_LIT } WidgetState;

// A raised panel with a soft shadow under it.
void widget_panel(App *a, Box b, float radius);

// A title bar style button with centered text. LIT keeps it highlighted
// (its dropdown is open, or it is the selected choice).
void widget_button(App *a, Box b, UiFont *f, const char *text, WidgetState state);

// The "Sure?" look of a button whose second click cannot be undone.
void widget_danger_button(App *a, Box b, UiFont *f, const char *text);

// on: 0 off .. 1 on; between, the knob is on its way.
void widget_switch(App *a, Box b, float on, bool hover);

// A round color swatch; the checked one gets a ring.
void widget_swatch(App *a, Box b, uint32_t color, bool checked, bool hover);

// Buttons joined into one rounded group. lit is the selected index (-1 for
// none) and hover the one under the mouse. A NULL text draws an inset value
// box instead of a button (the font size between A- and A+).
void widget_group(App *a, const Box *boxes, int n, UiFont *f, const char *const *texts, int lit,
                  int hover);

// A title in capitals (ASCII letters) with extra space between letters,
// like CSS text-transform: uppercase and letter-spacing. Returns its width.
float widget_spaced_text(App *a, UiFont *f, const char *text, float x, float y, float h, float spacing,
                        uint32_t color);
