// The pixel pictures of the terminal herons: the heron's poses, its chicks,
// and the marks over its head. Each row is a string, one character per
// sprite pixel, facing right:
//   . clear       W body    G wing and crest    K eye    Y beak and legs
//   C chick down  N call mark (! and ?)         Z snore  S sparkle
// ui/buddy_paint.c turns the letters into theme colors.
#pragma once

typedef enum {
  SPRITE_HERON_STAND,
  SPRITE_HERON_WALK_A,  // legs apart
  SPRITE_HERON_WALK_B,  // legs together
  SPRITE_HERON_PECK,    // head down at the ground
  SPRITE_HERON_FLY_UP,
  SPRITE_HERON_FLY_DOWN,
  SPRITE_HERON_SLEEP,  // on one leg, head tucked in
  SPRITE_HERON_CHEER,  // wings up
  SPRITE_CHICK_STAND,
  SPRITE_CHICK_HOP,
  SPRITE_CHICK_SLEEP,
  SPRITE_BANG,  // !
  SPRITE_ASK,   // ?
  SPRITE_SNORE,
  SPRITE_SPARK,
  SPRITE_COUNT
} SpriteId;

typedef struct {
  int w, h;
  const char *const *rows;
} Sprite;

// Every heron pose has this size, standing on its bottom row.
#define HERON_W 12
#define HERON_H 14
#define CHICK_W 6
#define CHICK_H 5
// The room a heron needs: its height and a mark over its head.
#define STAGE_H 21

const Sprite *sprite_get(SpriteId id);
