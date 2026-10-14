// Keeping a crew for each agent pane and drawing it.
#include "ui/buddies.h"

#include "base/log.h"
#include "base/mem.h"
#include "ui/agents.h"
#include "ui/buddy_paint.h"

// A heron with its mark over its head is this many rows tall.
#define STAGE_ROWS 3

// The size of one sprite pixel.
static float sprite_px(const Pane *p) { return STAGE_ROWS * (float)p->cell_h / STAGE_H; }

static PaneCrew *find(const BuddiesState *s, int pane) {
  for (int i = 0; i < s->count; i++) {
    if (s->crews[i].pane == pane) return &s->crews[i];
  }
  return NULL;
}

static PaneCrew *add(BuddiesState *s, int pane, ULONGLONG now) {
  if (s->count == s->cap) {
    s->cap = s->cap ? s->cap * 2 : 4;
    s->crews = xrealloc(s->crews, sizeof *s->crews * (size_t)s->cap);
  }
  PaneCrew *pc = &s->crews[s->count++];
  pc->pane = pane;
  crew_init(&pc->crew, (uint32_t)pane * 2654435761u ^ (uint32_t)now, now);
  log_debug("buddy", "pane %d: a heron comes", pane);
  return pc;
}

static void drop(BuddiesState *s, int i) {
  log_debug("buddy", "pane %d: the heron goes", s->crews[i].pane);
  s->crews[i] = s->crews[--s->count];
}

static bool shown(const App *a, int pane) {
  for (int i = 0; i < a->panes.shown.count; i++) {
    if (a->panes.shown.items[i]->id == pane) return true;
  }
  return false;
}

void buddies_step(App *a, ULONGLONG now) {
  BuddiesState *s = &a->buddies;
  bool on = s->on && a->motion;
  for (int i = s->count - 1; i >= 0; i--) {
    if (!on || !shown(a, s->crews[i].pane) || crew_done(&s->crews[i].crew)) drop(s, i);
  }
  if (!on) return;
  for (int i = 0; i < a->panes.shown.count; i++) {
    const Pane *p = a->panes.shown.items[i];
    const Agent *agent = agents_of(a, p);
    PaneCrew *pc = find(s, p->id);
    if ((!pc && !agent) || p->cell_w <= 0 || p->cell_h <= 0) continue;
    if (!pc) pc = add(s, p->id, now);
    float k = sprite_px(p);
    CrewInput in = {p->term,          agent != NULL,     agent ? agent->state : AGENT_IDLE,
                    agent ? agent->subagent_count : 0, (float)p->cell_w / k, (float)p->cell_h / k};
    crew_update(&pc->crew, &in, now);
  }
}

void buddies_paint(App *a, const Pane *p, ULONGLONG now) {
  const PaneCrew *pc = find(&a->buddies, p->id);
  if (!pc) return;
  Pose pose;
  crew_pose(&pc->crew, now, &pose);
  if (!pose.count) return;
  Box b = p->body;
  canvas_push_clip(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h);
  buddy_paint_pose(a->canvas, &pose, (float)b.x, (float)b.y, sprite_px(p));
  canvas_pop_clip(a->canvas);
}

unsigned buddies_wait(const App *a, ULONGLONG now) {
  unsigned wait = 0;
  if (IsIconic(a->hwnd)) return 0;  // nothing to see
  for (int i = 0; i < a->buddies.count; i++) {
    unsigned w = crew_wait(&a->buddies.crews[i].crew, now);
    if (w && (!wait || w < wait)) wait = w;
  }
  return wait;
}
