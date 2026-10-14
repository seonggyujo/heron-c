// Keeping a flock of herons for each agent pane and drawing it.
#include "ui/buddies.h"

#include "base/log.h"
#include "base/mem.h"
#include "ui/agents.h"
#include "ui/buddy_paint.h"

// A heron with its mark over its head is this many rows tall.
#define STAGE_ROWS 3

// The size of one sprite pixel.
static float sprite_px(const Pane *p) { return STAGE_ROWS * (float)p->cell_h / STAGE_H; }

static PaneFlock *find(const BuddiesState *s, int pane) {
  for (int i = 0; i < s->count; i++) {
    if (s->flocks[i].pane == pane) return &s->flocks[i];
  }
  return NULL;
}

static PaneFlock *add(BuddiesState *s, int pane) {
  if (s->count == s->cap) {
    s->cap = s->cap ? s->cap * 2 : 4;
    s->flocks = xrealloc(s->flocks, sizeof *s->flocks * (size_t)s->cap);
  }
  PaneFlock *f = &s->flocks[s->count++];
  f->pane = pane;
  f->count = 0;
  return f;
}

static void drop(BuddiesState *s, int i) {
  log_debug("buddy", "pane %d: the herons go", s->flocks[i].pane);
  s->flocks[i] = s->flocks[--s->count];
}

// Every heron of the flock has faded out.
static bool flock_done(const PaneFlock *f) {
  for (int i = 0; i < f->count; i++) {
    if (!crew_done(&f->crews[i])) return false;
  }
  return true;
}

static bool shown(const App *a, int pane) {
  for (int i = 0; i < a->panes.shown.count; i++) {
    if (a->panes.shown.items[i]->id == pane) return true;
  }
  return false;
}

// Moves p's herons to now: want of them stay while its agent does, and the
// rest fade out and go.
static void step_flock(PaneFlock *f, const Pane *p, const Agent *agent, int want, ULONGLONG now) {
  while (agent && f->count < want) {
    uint32_t seed = (uint32_t)p->id * 2654435761u ^ (uint32_t)f->count * 40503u ^ (uint32_t)now;
    crew_init(&f->crews[f->count], seed, f->count, now);
    log_debug("buddy", "pane %d: heron %d comes", p->id, f->count + 1);
    f->count++;
  }
  float k = sprite_px(p);
  for (int i = 0; i < f->count; i++) {
    bool stays = agent && i < want;
    CrewInput in = {p->term,
                    stays,
                    agent ? agent->state : AGENT_IDLE,
                    stays ? crew_chick_share(agent->subagent_count, want, i) : 0,
                    (float)p->cell_w / k,
                    (float)p->cell_h / k};
    crew_update(&f->crews[i], &in, now);
  }
  while (f->count > want && crew_done(&f->crews[f->count - 1])) f->count--;
}

void buddies_step(App *a, ULONGLONG now) {
  BuddiesState *s = &a->buddies;
  int want = a->motion ? s->herons : 0;
  for (int i = s->count - 1; i >= 0; i--) {
    if (!want || !shown(a, s->flocks[i].pane) || flock_done(&s->flocks[i])) drop(s, i);
  }
  if (!want) return;
  for (int i = 0; i < a->panes.shown.count; i++) {
    const Pane *p = a->panes.shown.items[i];
    const Agent *agent = agents_of(a, p);
    PaneFlock *f = find(s, p->id);
    if ((!f && !agent) || p->cell_w <= 0 || p->cell_h <= 0) continue;
    if (!f) f = add(s, p->id);
    step_flock(f, p, agent, want, now);
  }
}

void buddies_paint(App *a, const Pane *p, ULONGLONG now) {
  const PaneFlock *f = find(&a->buddies, p->id);
  if (!f || !f->count) return;
  Box b = p->body;
  float k = sprite_px(p);
  canvas_push_clip(a->canvas, (float)b.x, (float)b.y, (float)b.w, (float)b.h);
  for (int i = 0; i < f->count; i++) {
    Pose pose;
    crew_pose(&f->crews[i], now, &pose);
    buddy_paint_pose(a->canvas, &pose, (float)b.x, (float)b.y, k);
  }
  canvas_pop_clip(a->canvas);
}

unsigned buddies_wait(const App *a, ULONGLONG now) {
  unsigned wait = 0;
  if (IsIconic(a->hwnd)) return 0;  // nothing to see
  for (int i = 0; i < a->buddies.count; i++) {
    const PaneFlock *f = &a->buddies.flocks[i];
    for (int j = 0; j < f->count; j++) {
      unsigned w = crew_wait(&f->crews[j], now);
      if (w && (!wait || w < wait)) wait = w;
    }
  }
  return wait;
}
