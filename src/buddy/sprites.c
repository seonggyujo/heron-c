// The chicks and the marks, and finding any sprite by id.
#include "buddy/sprites.h"

#include "buddy/sprites_internal.h"

static const char *const CHICK_STAND[] = {".CCC..", "CCCKY.", "CCCCC.", ".CCC..", ".Y.Y.."};
static const char *const CHICK_HOP[] = {".CCC..", "CCCKY.", "CCCCC.", ".CCC..", "......"};
static const char *const CHICK_SLEEP[] = {"......", ".CCC..", "CCCCC.", "CCCCC.", ".CCC.."};

static const char *const BANG[] = {"NN", "NN", "NN", "NN", "..", "NN"};
static const char *const ASK[] = {".NN.", "N..N", "..N.", ".N..", "....", ".N.."};
static const char *const SNORE[] = {"ZZZZ", "..Z.", ".Z..", "ZZZZ"};
static const char *const SPARK[] = {"..S..", "..S..", "SSSSS", "..S..", "..S.."};

// As tall as it has rows.
#define SPRITE(w, rows) {w, (int)(sizeof rows / sizeof *rows), rows}
_Static_assert(sizeof CHICK_STAND / sizeof *CHICK_STAND == CHICK_H, "a chick is CHICK_H rows");

static const Sprite SMALL[SPRITE_COUNT - HERON_POSES] = {
    SPRITE(CHICK_W, CHICK_STAND),
    SPRITE(CHICK_W, CHICK_HOP),
    SPRITE(CHICK_W, CHICK_SLEEP),
    SPRITE(2, BANG),
    SPRITE(4, ASK),
    SPRITE(4, SNORE),
    SPRITE(5, SPARK),
};

const Sprite *sprite_get(SpriteId id) {
  return id < HERON_POSES ? &HERON_SPRITES[id] : &SMALL[id - HERON_POSES];
}
