// One pane's heron and its chicks. The heron stands on blank cells of the
// terminal (buddy/ground.h) and acts out the agent's state; each running
// subagent is a chick that follows it:
//   working     walks its run, stops to peck, now and then flies elsewhere
//   permission  calls with a !, flapping now and then (question, waiting: ?)
//   done        cheers with its wings up and a few sparks, then stands
//   idle        sleeps on one leg
// It flies in from the side when it first finds room, flies to the nearest
// room when text covers its spot, and fades out when there is none or the
// agent leaves. Pure: the caller gives the terminal, the agent and the
// time, and draws the poses. Positions are in sprite pixels from the
// terminal's top-left corner; a y is where feet stand.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "agent/agent_model.h"
#include "buddy/ground.h"
#include "buddy/sprites.h"
#include "vt/term.h"

#define CREW_CHICKS_MAX 6
#define CREW_PUFFS_MAX 8
// The heron, its mark, the chicks and the puffs.
#define CREW_ITEMS_MAX (2 + CREW_CHICKS_MAX + CREW_PUFFS_MAX)
// The frame time while something moves.
#define CREW_FRAME_MS 33

typedef enum { MOOD_GONE, MOOD_ASLEEP, MOOD_BUSY, MOOD_CALLING, MOOD_ASKING, MOOD_CHEERING } Mood;
typedef enum { ACT_WALK, ACT_PECK, ACT_PAUSE } Activity;

typedef struct {
  float x, y;
} Point;

typedef struct {
  const Term *term;
  bool present;  // the pane has an agent
  AgentState state;
  int subagents;
  float cell_w, cell_h;  // one cell, in sprite pixels
} CrewInput;

typedef struct {
  Point at;  // left edge and feet
  bool moving;
  uint64_t born;
} Chick;

// A spark that rises and fades: a cheer, or a chick leaving.
typedef struct {
  Point at;  // top-left at the start
  uint64_t start;
} Puff;

typedef struct {
  uint32_t rng;
  Mood mood;
  uint64_t mood_at, last;  // when the mood began; the last update
  float cell_w, cell_h;    // as last given
  bool placed;             // has a spot: stands there or flies to it
  Spot spot;
  Point at;  // the heron's left edge and feet; while flying, where it lands
  int dir;   // 1 faces right, -1 left
  bool flying;
  Point from;
  uint64_t fly_at;
  unsigned fly_ms;
  Activity act;
  uint64_t act_at, act_until, wander_at, retry_at;
  float walked;   // distance walked, for the stride
  float opacity;  // fades in and out
  int chick_count;
  Chick chicks[CREW_CHICKS_MAX];
  int puff_count;
  Puff puffs[CREW_PUFFS_MAX];
} Crew;

typedef struct {
  SpriteId sprite;
  float x, y;  // top-left
  bool flip;   // mirrored: faces left
  float opacity;
} PoseItem;

typedef struct {
  int count;
  PoseItem items[CREW_ITEMS_MAX];
} Pose;

void crew_init(Crew *c, uint32_t seed, uint64_t now);
// now is in ms, on any clock that only goes forward.
void crew_update(Crew *c, const CrewInput *in, uint64_t now);
// The agent left and everything has faded: the crew can go.
bool crew_done(const Crew *c);

// crew_pose.c
void crew_pose(const Crew *c, uint64_t now, Pose *out);
// How soon the next frame is needed, 0 for none (all stand still).
unsigned crew_wait(const Crew *c, uint64_t now);
