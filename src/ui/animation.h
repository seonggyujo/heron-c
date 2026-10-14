// The frame timer for the small animations (Heron's effects.css): the
// output flash on a pane border, the focus glow fading in and out, the
// status dot pulsing while output arrives, the empty state fading and its
// cursor blink, panes moving, appearing and leaving (ui/pane_motion.h), and
// the terminal herons (ui/buddies.h). The timer runs only while something
// moves, so an idle app draws no frames. Settings > Animations turns them
// off.
#pragma once

#include "ui/app.h"

// A pane got output. Throttled to one flash per 150 ms, as in Heron.
void anim_output(App *a, Pane *p);

// After each frame: asks for the next one, or stops the timer.
void anim_after_paint(App *a);

// How bright the border flash and the status dot of p are at now (0..1).
float anim_flash(const App *a, const Pane *p, unsigned long long now);
float anim_dot(const App *a, const Pane *p, unsigned long long now);
// How lit p's focus glow is (0..1): over 200 ms it comes on the pane that
// got focus and goes from the one that lost it.
float anim_glow(const App *a, const Pane *p, bool selected, unsigned long long now);
// How much of the empty state shows (0..1): it fades in over 200 ms from
// 120 ms after the last pane closed, and out over 200 ms when one opens.
float anim_empty(App *a, unsigned long long now);
