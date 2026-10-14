// Animation timing. Durations are Heron's: a 40 ms flash in and 300 ms
// out, the dot pulsing on a 1.2 s cycle for 3 s after the last output, and
// the empty-state cursor blinking every 550 ms.
#include "ui/animation.h"

#include <math.h>

#include "base/easing.h"
#include "ui/pane_motion.h"
#include "ui/popover.h"
#include "ui/sidebar.h"

#define THROTTLE_MS 150
#define FLASH_IN_MS 40
#define FLASH_OUT_MS 300
#define ACTIVE_MS 3000
#define PULSE_MS 1200
#define BLINK_MS 550
#define FRAME_MS 33
#define MOTION_FRAME_MS 15  // panes gliding or fading: as smooth as the timer allows
#define GLOW_MS 200
#define EMPTY_DELAY_MS 120
#define EMPTY_FADE_MS 200

void anim_output(App *a, Pane *p) {
  if (!a->motion) return;
  ULONGLONG now = GetTickCount64();
  if (now - p->output_at < THROTTLE_MS) return;
  p->output_at = now;
  anim_after_paint(a);
}

float anim_flash(const App *a, const Pane *p, unsigned long long now) {
  unsigned long long t = now - p->output_at;
  if (!a->motion || !p->output_at || t >= FLASH_IN_MS + FLASH_OUT_MS) return 0;
  return t < FLASH_IN_MS ? 1 : 1 - (float)(t - FLASH_IN_MS) / FLASH_OUT_MS;
}

float anim_dot(const App *a, const Pane *p, unsigned long long now) {
  unsigned long long t = now - p->output_at;
  if (!a->motion || !p->output_at || t >= ACTIVE_MS || !pane_running(p)) return 1;
  return 0.65f + 0.35f * cosf(2 * 3.14159265f * (float)(t % PULSE_MS) / PULSE_MS);
}

float anim_glow(const App *a, const Pane *p, bool selected, unsigned long long now) {
  unsigned long long t = now - a->focus_moved_at;
  float on = !a->motion || t >= GLOW_MS ? 1 : ease(EASE_OUT_CSS, (float)t / GLOW_MS);
  if (selected) return on;
  return p->id == a->unfocused_pane ? 1 - on : 0;
}

float anim_empty(App *a, unsigned long long now) {
  bool empty = a->panes.shown.count == 0;
  if (empty != a->empty_shown) {
    a->empty_shown = empty;
    a->empty_changed_at = now;
  }
  unsigned long long t = now - a->empty_changed_at;
  if (!a->motion) return empty;
  if (!empty) return t >= EMPTY_FADE_MS ? 0 : 1 - ease(EASE_OUT_CSS, (float)t / EMPTY_FADE_MS);
  if (t < EMPTY_DELAY_MS) return 0;
  t -= EMPTY_DELAY_MS;
  return t >= EMPTY_FADE_MS ? 1 : ease(EASE_OUT_CSS, (float)t / EMPTY_FADE_MS);
}

// The focus glow or the empty state is fading.
static bool fading(const App *a, ULONGLONG now) {
  return a->motion &&
         (now - a->focus_moved_at < GLOW_MS || now - a->empty_changed_at < EMPTY_DELAY_MS + EMPTY_FADE_MS);
}

void anim_after_paint(App *a) {
  ULONGLONG now = GetTickCount64();
  bool moving = motion_busy(a, now) || sidebar_busy(a, now) || popover_busy(a, now) || fading(a, now);
  UINT wait = moving ? MOTION_FRAME_MS : 0;
  const PaneArray *shown = &a->panes.shown;
  for (int i = 0; i < shown->count && a->motion && !wait; i++) {
    const Pane *p = shown->items[i];
    if (p->output_at && now - p->output_at < ACTIVE_MS && pane_running(p)) wait = FRAME_MS;
  }
  if (!wait && a->motion && shown->count == 0) wait = (UINT)(BLINK_MS - now % BLINK_MS) + 1;
  if (wait) {
    SetTimer(a->hwnd, TIMER_ANIMATION, wait, NULL);
  } else {
    KillTimer(a->hwnd, TIMER_ANIMATION);
  }
}
