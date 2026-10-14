// The crew's moods and its step through time.
#include "buddy/crew.h"

#include <math.h>

#include "base/log.h"
#include "buddy/crew_internal.h"

#define FADE_MS 300.0f
#define MAX_STEP 0.1f  // seconds: after a long still spell, move as if one frame passed

static const char *const MOOD_NAMES[] = {"gone", "asleep", "busy", "calling", "asking", "cheering"};

void crew_init(Crew *c, uint32_t seed, uint64_t now) {
  *c = (Crew){0};
  c->rng = seed ? seed : 1;
  c->mood = MOOD_GONE;
  c->mood_at = c->last = now;
  c->dir = -1;
}

uint32_t crew_random(Crew *c, uint32_t lo, uint32_t hi) {
  c->rng ^= c->rng << 13;  // xorshift32
  c->rng ^= c->rng >> 17;
  c->rng ^= c->rng << 5;
  return hi > lo ? lo + c->rng % (hi - lo) : lo;
}

bool crew_cheering(const Crew *c, uint64_t now) {
  return c->mood == MOOD_CHEERING && now - c->mood_at < CHEER_MS;
}

void crew_puff(Crew *c, Point at, uint64_t start) {
  if (c->puff_count < CREW_PUFFS_MAX) c->puffs[c->puff_count++] = (Puff){at, start};
}

static Mood mood_of(const CrewInput *in) {
  if (!in->present) return MOOD_GONE;
  switch (in->state) {
    case AGENT_WORKING: return MOOD_BUSY;
    case AGENT_PERMISSION: return MOOD_CALLING;
    case AGENT_QUESTION:
    case AGENT_WAITING: return MOOD_ASKING;
    case AGENT_DONE: return MOOD_CHEERING;
    default: return MOOD_ASLEEP;
  }
}

// Three sparks around the heron's head, one after another.
static void sparks(Crew *c, uint64_t now) {
  Point at = crew_heron_at(c, now);
  float top = at.y - HERON_H;
  crew_puff(c, (Point){at.x - 3, top + 2}, now);
  crew_puff(c, (Point){at.x + HERON_W / 2.0f - 1, top - 5}, now + 150);
  crew_puff(c, (Point){at.x + HERON_W, top}, now + 300);
}

static void set_mood(Crew *c, Mood mood, const CrewInput *in, uint64_t now) {
  if (mood == c->mood) return;
  log_debug("buddy", "heron %s -> %s", MOOD_NAMES[c->mood], MOOD_NAMES[mood]);
  c->mood = mood;
  c->mood_at = now;
  if (mood == MOOD_BUSY) {
    c->act = ACT_PAUSE;
    c->act_until = now;
    c->wander_at = now + crew_random(c, 6000, 12000);
  } else if (mood == MOOD_CHEERING && c->opacity > 0.01f) {
    sparks(c, now);
  } else if (mood == MOOD_CALLING || mood == MOOD_ASKING) {
    float middle = (float)term_cols(in->term) * in->cell_w / 2;  // it calls toward the middle
    c->dir = c->at.x + HERON_W / 2.0f > middle ? -1 : 1;
  }
}

static void drop_old_puffs(Crew *c, uint64_t now) {
  int kept = 0;
  for (int i = 0; i < c->puff_count; i++) {
    if (now < c->puffs[i].start + PUFF_MS) c->puffs[kept++] = c->puffs[i];
  }
  c->puff_count = kept;
}

void crew_update(Crew *c, const CrewInput *in, uint64_t now) {
  float dt = now > c->last ? fminf((float)(now - c->last) / 1000.0f, MAX_STEP) : 0;
  c->last = now;
  c->cell_w = in->cell_w;
  c->cell_h = in->cell_h;
  set_mood(c, mood_of(in), in, now);
  if (c->flying && now >= c->fly_at + c->fly_ms) c->flying = false;  // landed
  // Mid-frame (synchronized output) the screen may be half redrawn: the
  // spot is judged on the finished frame.
  if (c->mood != MOOD_GONE && !term_modes(in->term).synchronized) crew_keep_spot(c, in, now);
  bool shown = c->placed && c->mood != MOOD_GONE;
  float fade = dt * 1000.0f / FADE_MS;
  c->opacity = shown ? fminf(1, c->opacity + fade) : fmaxf(0, c->opacity - fade);
  if (shown && !c->flying && c->mood == MOOD_BUSY) crew_busy(c, in, dt, now);
  chicks_update(c, shown ? in->subagents : 0, dt, now);
  drop_old_puffs(c, now);
}

bool crew_done(const Crew *c) {
  return c->mood == MOOD_GONE && c->opacity <= 0 && !c->chick_count && !c->puff_count;
}
