// The chicks, one per running subagent, dealt out among the pane's herons:
// they hatch beside their heron, follow in a line behind it inside its run,
// and leave with a spark.
#include <math.h>

#include "buddy/crew_internal.h"

#define CHICK_GAP 7.0f  // sprite pixels between chicks in the line
#define FOLLOW 6.0f     // how much of the way they catch up in a second

static void hatch(Crew *c, uint64_t now) {
  Point heron = crew_heron_at(c, now);
  Chick *k = &c->chicks[c->chick_count++];
  *k = (Chick){{heron.x + (HERON_W - CHICK_W) / 2.0f, heron.y}, false, now};
}

static void leave(Crew *c, uint64_t now) {
  const Chick *k = &c->chicks[--c->chick_count];
  crew_puff(c, (Point){k->at.x + 1, k->at.y - CHICK_H}, now);
}

// Where chick i wants to be: in line behind the heron, inside its run.
static Point place_of(const Crew *c, int i, Point heron) {
  float x = c->dir > 0 ? heron.x - (float)(i + 1) * CHICK_GAP : heron.x + HERON_W + 1 + (float)i * CHICK_GAP;
  if (!c->flying) {
    float left = (float)c->spot.c0 * c->cell_w, right = (float)c->spot.c1 * c->cell_w - CHICK_W;
    x = x < left ? left : x > right ? right : x;
  }
  return (Point){x, heron.y};
}

int crew_chick_share(int subagents, int herons, int index) {
  if (subagents <= 0 || herons <= 0 || index < 0 || index >= herons) return 0;
  return (subagents - index + herons - 1) / herons;
}

void chicks_update(Crew *c, int want, float dt, uint64_t now) {
  if (want > CREW_CHICKS_MAX) want = CREW_CHICKS_MAX;
  if (want < 0) want = 0;
  while (c->chick_count < want) hatch(c, now);
  while (c->chick_count > want) leave(c, now);
  Point heron = crew_heron_at(c, now);
  float pull = fminf(1.0f, dt * FOLLOW);
  for (int i = 0; i < c->chick_count; i++) {
    Chick *k = &c->chicks[i];
    Point to = place_of(c, i, heron);
    k->moving = fabsf(to.x - k->at.x) > 0.6f || fabsf(to.y - k->at.y) > 0.6f;
    k->at.x += (to.x - k->at.x) * pull;
    k->at.y += (to.y - k->at.y) * pull;
  }
}
