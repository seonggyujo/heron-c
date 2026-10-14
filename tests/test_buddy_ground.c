// Tests for the terminal herons' sprites and for finding blank room in a
// terminal.
#include <string.h>

#include "buddy/ground.h"
#include "buddy/sprites.h"
#include "check.h"
#include "term_helpers.h"

void test_buddy_sprites(void) {
  for (int id = 0; id < SPRITE_COUNT; id++) {
    const Sprite *s = sprite_get((SpriteId)id);
    for (int r = 0; r < s->h; r++) CHECK((int)strlen(s->rows[r]) == s->w);
  }
}

void test_buddy_ground(void) {
  Term *t = term_new(40, 10, 0);
  feed(t, "\x1b[?25l");
  Spot s;
  CHECK(ground_find(t, 4, 2, FIND_HOME, 0, 0, 0, &s));
  CHECK(s.row == 9 && s.c0 == 0 && s.c1 == 40);  // an empty screen: its bottom
  CHECK(ground_find(t, 4, 2, FIND_ANY, 0, 0, 3, &s) && s.row == 4);  // the fourth of nine
  feed(t, "\x1b[10;1Hhello");
  CHECK(ground_find(t, 4, 2, FIND_HOME, 0, 0, 0, &s));
  CHECK(s.row == 9 && s.c0 == 5 && s.c1 == 40);  // beside the text
  CHECK(!ground_free(t, 4, 9) && ground_free(t, 5, 9));
  CHECK(ground_run(t, 9, 2, 6, 8, &s) && s.c0 == 5 && s.c1 == 40);
  CHECK(!ground_run(t, 9, 2, 3, 6, &s));
  // A colored background and a reverse-video space are not blank.
  feed(t, "\x1b[5;3H\x1b[41m \x1b[0m\x1b[7m \x1b[0m");
  CHECK(!ground_free(t, 2, 4) && !ground_free(t, 3, 4) && ground_free(t, 4, 4));
  // Nor are the cursor and the cells right after it.
  feed(t, "\x1b[?25h\x1b[3;10H");
  CHECK(!ground_free(t, 9, 2) && !ground_free(t, 11, 2) && ground_free(t, 12, 2));
  term_free(t);

  t = term_new(40, 10, 0);
  feed(t, "\x1b[?25l\x1b[6;1H");
  for (int i = 0; i < 40; i++) feed(t, "x");  // a full row 5 splits the screen
  CHECK(ground_find(t, 4, 2, FIND_NEAR, 0, 2, 0, &s) && s.row == 2);
  CHECK(ground_find(t, 4, 2, FIND_NEAR, 0, 5, 0, &s) && s.row == 4);
  CHECK(ground_find(t, 4, 2, FIND_NEAR, 0, 8, 0, &s) && s.row == 8);
  term_free(t);

  t = term_new(10, 3, 0);
  feed(t, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
  CHECK(!ground_find(t, 3, 2, FIND_HOME, 0, 0, 0, &s));  // no room anywhere
  term_free(t);
}
