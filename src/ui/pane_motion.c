// Pane glide, enter and leave animations.
#include "ui/pane_motion.h"

#include <math.h>
#include <stdlib.h>

#include "base/easing.h"
#include "base/mem.h"

#define GLIDE_MS 220
#define ENTER_MS 220
#define LEAVE_MS 180

// How far along an animation that started at since is (0..1).
static float progress(ULONGLONG since, ULONGLONG now, int ms) {
  float t = (float)(now - since) / ms;
  return t < 1 ? t : 1;
}

static bool running(ULONGLONG since, ULONGLONG now, int ms) { return since && now - since < (ULONGLONG)ms; }

static PanePose scaled(PanePose p, float scale) {
  float cx = p.x + p.w / 2, cy = p.y + p.h / 2;
  p.w *= scale;
  p.h *= scale;
  p.x = cx - p.w / 2;
  p.y = cy - p.h / 2;
  return p;
}

PanePose motion_pose(const App *a, const Pane *p, ULONGLONG now) {
  Box f = p->frame;
  PanePose pose = {(float)f.x, (float)f.y, (float)f.w, (float)f.h, 1};
  if (!a->motion) return pose;
  if (running(p->moved_at, now, GLIDE_MS)) {
    float e = ease(EASE_OUT, progress(p->moved_at, now, GLIDE_MS));
    Box b = p->moved_from;
    pose.x = b.x + (f.x - b.x) * e;
    pose.y = b.y + (f.y - b.y) * e;
    pose.w = b.w + (f.w - b.w) * e;
    pose.h = b.h + (f.h - b.h) * e;
  }
  if (p->closed_at) {
    float e = ease(EASE_IN, progress(p->closed_at, now, LEAVE_MS));
    pose = scaled(pose, 1 - 0.06f * e);
    pose.opacity = 1 - e;
  } else if (running(p->opened_at, now, ENTER_MS)) {
    float e = ease(EASE_OUT, progress(p->opened_at, now, ENTER_MS));
    pose = scaled(pose, 0.96f + 0.04f * e);
    pose.opacity = e;
  }
  return pose;
}

void motion_capture(const App *a, MotionSnapshot *s) {
  const PaneArray *shown = &a->panes.shown;
  ULONGLONG now = GetTickCount64();
  s->count = shown->count;
  s->ids = xmalloc(sizeof(int) * (size_t)(s->count + 1));
  s->boxes = xmalloc(sizeof(Box) * (size_t)(s->count + 1));
  for (int i = 0; i < s->count; i++) {
    PanePose pose = motion_pose(a, shown->items[i], now);
    s->ids[i] = shown->items[i]->id;
    s->boxes[i] = (Box){(int)lroundf(pose.x), (int)lroundf(pose.y), (int)lroundf(pose.w), (int)lroundf(pose.h)};
  }
}

static bool same_box(Box a, Box b) { return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h; }

void motion_glide(App *a, MotionSnapshot *s) {
  ULONGLONG now = GetTickCount64();
  for (int i = 0; i < s->count && a->motion; i++) {
    Pane *p = panes_find(&a->panes, s->ids[i]);
    if (!p || p->closed_at || same_box(p->frame, s->boxes[i]) || s->boxes[i].w <= 0) continue;
    p->moved_from = s->boxes[i];
    p->moved_at = now;
  }
  free(s->ids);
  free(s->boxes);
  s->ids = NULL;
  s->boxes = NULL;
}

void motion_enter(App *a, Pane *p) {
  if (a->motion) p->opened_at = GetTickCount64();
}

void motion_leave(App *a, Pane *p) { p->closed_at = GetTickCount64(); }

bool motion_left(const App *a, const Pane *p, ULONGLONG now) {
  return !a->motion || !running(p->closed_at, now, LEAVE_MS);
}

static bool array_busy(const PaneArray *panes, ULONGLONG now) {
  for (int i = 0; i < panes->count; i++) {
    const Pane *p = panes->items[i];
    if (running(p->moved_at, now, GLIDE_MS) || running(p->opened_at, now, ENTER_MS)) return true;
  }
  return false;
}

bool motion_busy(const App *a, ULONGLONG now) {
  if (a->panes.leaving.count) return true;  // each must be released when done
  return a->motion && array_busy(&a->panes.shown, now);
}
