// CSS timing curves for the animations: a cubic Bezier from (0, 0) to
// (1, 1) through two control points, as in cubic-bezier(x1, y1, x2, y2).
#pragma once

typedef struct {
  float x1, y1, x2, y2;
} Easing;

// Heron's EASE_OUT, cubic-bezier(0.2, 0.8, 0.2, 1): fast start, soft landing.
#define EASE_OUT ((Easing){0.2f, 0.8f, 0.2f, 1.0f})
// CSS ease-in and ease-out.
#define EASE_IN ((Easing){0.42f, 0.0f, 1.0f, 1.0f})
#define EASE_OUT_CSS ((Easing){0.0f, 0.0f, 0.58f, 1.0f})

// Progress at time t (0..1, clamped) along the curve.
float ease(Easing e, float t);
