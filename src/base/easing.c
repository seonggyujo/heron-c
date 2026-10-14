// Cubic Bezier timing curves.
#include "base/easing.h"

#include <math.h>

// One coordinate of the curve at parameter s, for control values a and b.
static float bezier(float a, float b, float s) {
  float r = 1 - s;
  return 3 * r * r * s * a + 3 * r * s * s * b + s * s * s;
}

static float bezier_slope(float a, float b, float s) {
  float r = 1 - s;
  return 3 * r * r * a + 6 * r * s * (b - a) + 3 * s * s * (1 - b);
}

float ease(Easing e, float t) {
  if (t <= 0) return 0;
  if (t >= 1) return 1;
  // Find s where x(s) = t: Newton steps, then halving if the slope is flat.
  float s = t;
  for (int i = 0; i < 8; i++) {
    float slope = bezier_slope(e.x1, e.x2, s);
    if (fabsf(slope) < 1e-6f) break;
    s -= (bezier(e.x1, e.x2, s) - t) / slope;
  }
  if (s < 0 || s > 1 || fabsf(bezier(e.x1, e.x2, s) - t) > 1e-4f) {
    float lo = 0, hi = 1;
    for (int i = 0; i < 30; i++) {
      s = (lo + hi) / 2;
      if (bezier(e.x1, e.x2, s) < t) lo = s;
      else hi = s;
    }
  }
  return bezier(e.y1, e.y2, s);
}
