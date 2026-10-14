// How the heron gets around: keeping to blank cells, flying to new room,
// and walking and pecking while the agent works.
#include <math.h>

#include "base/log.h"
#include "buddy/crew_internal.h"

#define WALK_SPEED 14.0f  // sprite pixels a second
#define RETRY_MS 250      // with no room, how often to look again
// Cells kept between the heron and the ends of its run when there is room:
// text beside a run tends to grow (longer lines scroll in) and cover it.
#define EDGE_ROOM 2
// Runs this many cells wider than the heron are tried first.
#define ROOMY (2 * EDGE_ROOM + 1)
#define LINE_GAP (HERON_W + 6.0f)  // between herons that fly in together

static int rows_tall(const CrewInput *in) { return (int)ceilf(STAGE_H / in->cell_h - 0.01f); }
static int cols_wide(const CrewInput *in) { return (int)ceilf(HERON_W / in->cell_w - 0.01f); }

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

// How far the heron keeps from each end of its run.
static float edge_room(const Crew *c) {
  float slack = (float)(c->spot.c1 - c->spot.c0) * c->cell_w - HERON_W;
  return slack > 0 ? fminf(EDGE_ROOM * c->cell_w, slack / 2) : 0;
}

// The heron's left edge at the two ends of its run, a little way in from
// the text beside it.
static float crew_left(const Crew *c) { return (float)c->spot.c0 * c->cell_w + edge_room(c); }
static float crew_right(const Crew *c) {
  float right = (float)c->spot.c1 * c->cell_w - HERON_W - edge_room(c);
  return right > crew_left(c) ? right : crew_left(c);
}

// A roomy run first, else any run the heron fits in.
static bool find_room(const CrewInput *in, int extra, FindMode mode, int col, int row, uint32_t pick, Spot *out) {
  int w = cols_wide(in) + extra, h = rows_tall(in);
  return ground_find(in->term, w + ROOMY, h, mode, col, row, pick, out) ||
         ground_find(in->term, w, h, mode, col, row, pick, out);
}

// The run under a heron standing at `at`.
static bool run_under(const CrewInput *in, Point at, Spot *out) {
  int row = (int)lroundf(at.y / in->cell_h) - 1;
  int col0 = (int)floorf(at.x / in->cell_w + 0.01f);
  int col1 = (int)floorf((at.x + HERON_W) / in->cell_w - 0.01f);
  return ground_run(in->term, row, rows_tall(in), col0, col1, out);
}

Point crew_heron_at(const Crew *c, uint64_t now) {
  if (!c->flying || !c->fly_ms) return c->at;
  float t = clampf((float)(now - c->fly_at) / (float)c->fly_ms, 0, 1);
  float e = t * t * (3 - 2 * t);
  float dx = c->at.x - c->from.x, dy = c->at.y - c->from.y;
  float arc = fminf(hypotf(dx, dy) * 0.3f, 18.0f) + 4.0f;
  Point p = {c->from.x + dx * e, c->from.y + dy * e - arc * 4 * t * (1 - t)};
  return p;
}

// Flies to s from where the heron is, or in from the nearer side when it
// was not to be seen, landing a heron's width further in for each heron
// of the pane before it.
static void fly_to(Crew *c, const CrewInput *in, Spot s, uint64_t now) {
  Point from = crew_heron_at(c, now);
  c->spot = s;
  c->placed = true;
  Point to = {clampf(from.x, crew_left(c), crew_right(c)), (float)(s.row + 1) * in->cell_h};
  if (c->opacity <= 0.01f) {
    float width = (float)term_cols(in->term) * in->cell_w;
    bool right = to.x + HERON_W / 2.0f > width / 2;
    from = (Point){right ? width + 2 : -HERON_W - 2.0f, to.y - 2 * in->cell_h};
    float in_by = (float)c->index * LINE_GAP;
    to.x = clampf(right ? crew_right(c) - in_by : crew_left(c) + in_by, crew_left(c), crew_right(c));
    c->opacity = 1;
  }
  float dist = hypotf(to.x - from.x, to.y - from.y);
  if (fabsf(to.x - from.x) > 0.5f) c->dir = to.x > from.x ? 1 : -1;
  c->from = from;
  c->at = to;
  c->flying = dist >= 1;
  c->fly_at = now;
  c->fly_ms = (unsigned)clampf(dist * 9, 350, 1100);
  log_debug("buddy", "heron flies to row %d, columns %d-%d", s.row, s.c0, s.c1 - 1);
}

void crew_keep_spot(Crew *c, const CrewInput *in, uint64_t now) {
  if (c->flying) return;  // looked at again once it lands
  Spot s;
  if (c->placed && run_under(in, c->at, &s)) {
    c->spot = s;
    return;
  }
  if (now < c->retry_at) return;
  int col = (int)(c->at.x / in->cell_w), row = (int)lroundf(c->at.y / in->cell_h) - 1;
  FindMode mode = c->opacity > 0.01f ? FIND_NEAR : FIND_HOME;
  if (find_room(in, 1, mode, col, row, 0, &s)) {
    fly_to(c, in, s, now);
    return;
  }
  if (c->placed) log_debug("buddy", "no room for the heron");
  c->placed = false;
  c->retry_at = now + RETRY_MS;
}

// Now and then the busy heron flies to another run with room to walk.
static void wander(Crew *c, const CrewInput *in, uint64_t now) {
  c->wander_at = now + crew_random(c, 9000, 16000);
  Spot s;
  uint32_t pick = crew_random(c, 0, 1u << 30);
  if (!find_room(in, 3, FIND_ANY, 0, 0, pick, &s)) return;
  if (s.row == c->spot.row && s.c0 == c->spot.c0) return;  // its own run
  fly_to(c, in, s, now);
}

// Walking, then pecking or a pause, then walking again, maybe turned round.
static void next_act(Crew *c, uint64_t now) {
  static const uint32_t SHORTEST[] = {1500, 700, 400}, LONGEST[] = {4000, 1300, 900};
  if (c->act == ACT_WALK) {
    c->act = crew_random(c, 0, 10) < 6 ? ACT_PECK : ACT_PAUSE;
  } else {
    c->act = ACT_WALK;
    if (crew_random(c, 0, 10) < 3) c->dir = -c->dir;
  }
  c->act_at = now;
  c->act_until = now + crew_random(c, SHORTEST[c->act], LONGEST[c->act]);
}

void crew_busy(Crew *c, const CrewInput *in, float dt, uint64_t now) {
  if (now >= c->wander_at) {
    wander(c, in, now);
    if (c->flying) return;
  }
  if (now >= c->act_until) next_act(c, now);
  if (c->act != ACT_WALK) return;
  // It turns at the ends of its run. When the ends moved past it (text grew
  // without covering it), it turns where it is instead of jumping.
  float step = WALK_SPEED * dt, left = crew_left(c), right = crew_right(c);
  float next = c->at.x + (float)c->dir * step;
  if (c->dir > 0 && next > right) {
    next = fmaxf(c->at.x, right);
    c->dir = -1;
  } else if (c->dir < 0 && next < left) {
    next = fminf(c->at.x, left);
    c->dir = 1;
  }
  c->walked += fabsf(next - c->at.x);
  c->at.x = next;
}
