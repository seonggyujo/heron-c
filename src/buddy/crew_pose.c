// What the crew looks like at a moment, and when that next changes.
#include <math.h>

#include "buddy/crew_internal.h"

#define WING_MS 120u   // a flight's wing beat
#define CHEER_BEAT 150u
#define PECK_MS 220u
#define STRIDE 4.0f    // sprite pixels walked per step
#define HATCH_MS 200u  // a new chick fades in

static void add(Pose *p, SpriteId s, float x, float y, bool flip, float opacity) {
  if (p->count == CREW_ITEMS_MAX || opacity <= 0.01f) return;
  p->items[p->count++] = (PoseItem){s, x, y, flip, opacity};
}

static SpriteId heron_sprite(const Crew *c, uint64_t now) {
  if (c->flying) return now / WING_MS % 2 ? SPRITE_HERON_FLY_UP : SPRITE_HERON_FLY_DOWN;
  switch (c->mood) {
    case MOOD_ASLEEP: return SPRITE_HERON_SLEEP;
    case MOOD_CHEERING: return crew_cheering(c, now) && now / CHEER_BEAT % 2 ? SPRITE_HERON_CHEER : SPRITE_HERON_STAND;
    case MOOD_CALLING:
    case MOOD_ASKING:  // a flap every fourth beat
      return (now - c->mood_at) / CALL_MS % 4 == 3 ? SPRITE_HERON_CHEER : SPRITE_HERON_STAND;
    case MOOD_BUSY:
      if (c->act == ACT_WALK) return (int)(c->walked / STRIDE) % 2 ? SPRITE_HERON_WALK_A : SPRITE_HERON_WALK_B;
      if (c->act == ACT_PECK) return (now - c->act_at) / PECK_MS % 2 ? SPRITE_HERON_STAND : SPRITE_HERON_PECK;
      return SPRITE_HERON_STAND;
    default: return SPRITE_HERON_STAND;
  }
}

// How high a cheering heron hops at now.
static float hop(const Crew *c, uint64_t now) {
  if (c->flying || !crew_cheering(c, now)) return 0;
  return fabsf(sinf((float)(now - c->mood_at) * 0.0045f)) * 5.0f;
}

// Over its head: ! or ? while it calls (bobbing), z while it sleeps (rising
// once a second).
static void add_mark(Pose *p, const Crew *c, Point at, uint64_t now) {
  if (c->flying) return;
  bool left = c->dir < 0;
  if (c->mood == MOOD_CALLING || c->mood == MOOD_ASKING) {
    SpriteId id = c->mood == MOOD_CALLING ? SPRITE_BANG : SPRITE_ASK;
    const Sprite *s = sprite_get(id);
    float head = left ? HERON_W - 6.5f : 6.5f;
    float bob = (now - c->mood_at) / CALL_MS % 2 ? 1.0f : 0.0f;
    add(p, id, at.x + head - (float)s->w / 2, at.y - HERON_H - (float)s->h - bob, false, c->opacity);
  } else if (c->mood == MOOD_ASLEEP) {
    float rise = (float)(now / 1000 % 3) * 2;
    add(p, SPRITE_SNORE, at.x + (left ? 0.0f : HERON_W - 4.0f), at.y - 11 - rise, false, c->opacity);
  }
}

static void add_chicks(Pose *p, const Crew *c, uint64_t now) {
  bool cheer = crew_cheering(c, now);
  for (int i = 0; i < c->chick_count; i++) {
    const Chick *k = &c->chicks[i];
    float lift = k->moving || cheer ? fabsf(sinf((float)now * 0.014f + (float)i)) * 2.5f : 0;
    SpriteId s = c->mood == MOOD_ASLEEP ? SPRITE_CHICK_SLEEP : lift > 1 ? SPRITE_CHICK_HOP : SPRITE_CHICK_STAND;
    float hatched = fminf(1.0f, (float)(now - k->born) / HATCH_MS);
    add(p, s, k->at.x, k->at.y - CHICK_H - lift, c->dir < 0, c->opacity * hatched);
  }
}

void crew_pose(const Crew *c, uint64_t now, Pose *out) {
  out->count = 0;
  Point at = crew_heron_at(c, now);
  at.y -= hop(c, now);
  add(out, heron_sprite(c, now), at.x, at.y - HERON_H, c->dir < 0, c->opacity);
  add_mark(out, c, at, now);
  add_chicks(out, c, now);
  for (int i = 0; i < c->puff_count; i++) {
    const Puff *f = &c->puffs[i];
    if (now < f->start) continue;
    float t = (float)(now - f->start) / PUFF_MS;
    add(out, SPRITE_SPARK, f->at.x, f->at.y - t * 4, false, 1 - t);
  }
}

static bool chicks_busy(const Crew *c, uint64_t now) {
  for (int i = 0; i < c->chick_count; i++) {
    if (c->chicks[i].moving || now - c->chicks[i].born < HATCH_MS) return true;
  }
  return false;
}

unsigned crew_wait(const Crew *c, uint64_t now) {
  bool shown = c->placed && c->mood != MOOD_GONE;
  bool fading = shown ? c->opacity < 1 : c->opacity > 0;
  if (fading || c->flying || c->puff_count || chicks_busy(c, now)) return CREW_FRAME_MS;
  if (!shown) return 0;
  switch (c->mood) {
    case MOOD_BUSY: return CREW_FRAME_MS;
    case MOOD_CHEERING: return crew_cheering(c, now) ? CREW_FRAME_MS : 0;
    case MOOD_CALLING:
    case MOOD_ASKING: return CALL_MS - (unsigned)((now - c->mood_at) % CALL_MS);
    default: return 0;  // asleep: the z moves with the 1 s clock frames
  }
}
