// Drop zones on a pane.
#include "layout/drop_zone.h"

#define EDGE 0.25

Side drop_zone_at(Box b, int x, int y) {
  double fx = (double)(x - b.x) / b.w, fy = (double)(y - b.y) / b.h;
  if (fx > EDGE && fx < 1 - EDGE && fy > EDGE && fy < 1 - EDGE) return SIDE_CENTER;
  double dist[4] = {fx, 1 - fx, fy, 1 - fy};  // left, right, top, bottom
  int best = 0;
  for (int i = 1; i < 4; i++) {
    if (dist[i] < dist[best]) best = i;
  }
  return (Side)best;
}

Box drop_preview(Box b, Side zone) {
  switch (zone) {
    case SIDE_LEFT: return (Box){b.x, b.y, b.w / 2, b.h};
    case SIDE_RIGHT: return (Box){b.x + b.w / 2, b.y, b.w - b.w / 2, b.h};
    case SIDE_TOP: return (Box){b.x, b.y, b.w, b.h / 2};
    case SIDE_BOTTOM: return (Box){b.x, b.y + b.h / 2, b.w, b.h - b.h / 2};
    default: return b;
  }
}
