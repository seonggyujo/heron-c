// The heron's poses, 12 by 14 sprite pixels, facing right, feet on the
// bottom row.
#include "buddy/sprites_internal.h"

// The head, the S of the neck and the body, the same while standing and
// walking.
#define UPRIGHT       \
  "......WW....",     \
  "....GWWKYYYY",     \
  "......WW....",     \
  ".......W....",     \
  ".......W....",     \
  "......WW....",     \
  "......W.....",     \
  "..GGGWWW....",     \
  ".GGGGWWWW...",     \
  "GGGGGGWWW...",     \
  "..GGGGWW...."

#define EMPTY_7 "............", "............", "............", "............", \
                "............", "............", "............"

static const char *const STAND[] = {UPRIGHT, ".....Y.Y....", ".....Y.Y....", "....YY.YY..."};
static const char *const WALK_A[] = {UPRIGHT, ".....Y..Y...", "....Y....Y..", "...YY....YY."};
static const char *const WALK_B[] = {UPRIGHT, "......Y.....", "......Y.....", ".....YYY...."};

static const char *const PECK[] = {
    EMPTY_7,
    "..GGGWW.....",
    ".GGGGWWWW...",
    "GGGGGGWWWW..",
    "..GGGGW..WW.",
    ".....Y.Y..WK",
    ".....Y.Y..WY",
    "....YY.YY..Y",
};

static const char *const FLY_UP[] = {
    "............", "............", "............",
    "....GG......",
    "....GGG.....",
    ".....GGG....",
    ".....GGG.WW.",
    "..WWWWWWWWKY",
    "YYYWWWWWWW..",
    "....WWWW....",
    "............", "............", "............", "............",
};

static const char *const FLY_DOWN[] = {
    "............", "............", "............", "............", "............", "............",
    ".........WW.",
    "..WWWWWWWWKY",
    "YYYWWWWWWW..",
    "....WGGGW...",
    ".....GGG....",
    ".....GG.....",
    "......G.....",
    "............",
};

static const char *const SLEEP[] = {
    EMPTY_7,
    "......WWW...",
    "..GGGWGWY...",
    ".GGGGWWWW...",
    "GGGGGGWWW...",
    "..GGGGWW....",
    "......Y.....",
    ".....YYY....",
};

static const char *const CHEER[] = {
    "......WW....",
    "G...GWWKYYYY",
    "GG....WW....",
    ".GG....W....",
    ".GGG...W....",
    "..GGG.WW....",
    "...GG.W.....",
    "..GGGWWW....",
    ".GGGGWWWW...",
    "..GGGGWWW...",
    "....GGWW....",
    ".....Y.Y....",
    ".....Y.Y....",
    "....YY.YY...",
};

#define POSE(rows) {HERON_W, HERON_H, rows}
#define TALL(rows) _Static_assert(sizeof rows / sizeof *rows == HERON_H, #rows " is not HERON_H rows")
TALL(STAND);
TALL(WALK_A);
TALL(WALK_B);
TALL(PECK);
TALL(FLY_UP);
TALL(FLY_DOWN);
TALL(SLEEP);
TALL(CHEER);

const Sprite HERON_SPRITES[HERON_POSES] = {
    [SPRITE_HERON_STAND] = POSE(STAND),       [SPRITE_HERON_WALK_A] = POSE(WALK_A),
    [SPRITE_HERON_WALK_B] = POSE(WALK_B),     [SPRITE_HERON_PECK] = POSE(PECK),
    [SPRITE_HERON_FLY_UP] = POSE(FLY_UP),     [SPRITE_HERON_FLY_DOWN] = POSE(FLY_DOWN),
    [SPRITE_HERON_SLEEP] = POSE(SLEEP),       [SPRITE_HERON_CHEER] = POSE(CHEER),
};
