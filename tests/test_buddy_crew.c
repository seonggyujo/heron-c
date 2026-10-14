// Tests for a terminal heron's crew: its moods, moves and chicks.
#include <stdio.h>

#include "buddy/crew.h"
#include "check.h"
#include "term_helpers.h"

// A cell in sprite pixels, as ui/buddies.c sizes them: a heron and its mark
// fill three rows (an 8 by 17 pixel cell).
#define CELL_W 3.3f
#define CELL_H 7.0f

// Runs the crew for ms in 33 ms frames.
static void run(Crew *c, const CrewInput *in, uint64_t *now, unsigned ms) {
  for (unsigned t = 0; t < ms; t += 33) {
    *now += 33;
    crew_update(c, in, *now);
  }
}

// Every cell the standing heron covers is blank.
static bool on_blank(const Crew *c, const CrewInput *in) {
  int row = (int)(c->at.y / in->cell_h + 0.5f) - 1;
  int col0 = (int)(c->at.x / in->cell_w + 0.01f), col1 = (int)((c->at.x + HERON_W) / in->cell_w - 0.01f);
  for (int y = row - 2; y <= row; y++) {
    for (int x = col0; x <= col1; x++) {
      if (!ground_free(in->term, x, y)) return false;
    }
  }
  return true;
}

static int count_sprites(const Crew *c, uint64_t now, SpriteId from, SpriteId to) {
  Pose pose;
  crew_pose(c, now, &pose);
  int n = 0;
  for (int i = 0; i < pose.count; i++) n += pose.items[i].sprite >= from && pose.items[i].sprite <= to;
  return n;
}

static void fill_rows(Term *t, int first, int last) {
  char move[24];
  for (int y = first; y <= last; y++) {
    snprintf(move, sizeof move, "\x1b[%d;1H", y + 1);
    feed(t, move);
    for (int i = 0; i < 60; i++) feed(t, "x");
  }
}

void test_buddy_crew(void) {
  Term *t = term_new(60, 12, 0);
  feed(t, "\x1b[?25l");
  Crew c;
  uint64_t now = 1000;
  crew_init(&c, 7, now);
  CrewInput in = {t, true, AGENT_WORKING, 0, CELL_W, CELL_H};
  crew_update(&c, &in, now);
  CHECK(c.mood == MOOD_BUSY && c.placed && c.flying && c.spot.row == 11);  // flies in, to the bottom
  run(&c, &in, &now, 1500);
  CHECK(!c.flying && c.opacity == 1 && on_blank(&c, &in));
  CHECK(crew_wait(&c, now) == CREW_FRAME_MS);
  float x = c.at.x;
  run(&c, &in, &now, 500);
  CHECK(c.at.x != x);  // walking

  feed(t, "\x1b[?2026h");  // mid-frame (synchronized output): judged later
  fill_rows(t, 10, 11);    // text covers it
  run(&c, &in, &now, 33);
  CHECK(!c.flying && c.spot.row == 11);
  feed(t, "\x1b[?2026l");  // the frame is done: it flies to the nearest room
  run(&c, &in, &now, 33);
  CHECK(c.flying && c.spot.row == 9);
  run(&c, &in, &now, 1500);
  CHECK(!c.flying && on_blank(&c, &in));

  in.subagents = 2;
  run(&c, &in, &now, 1000);
  CHECK(c.chick_count == 2 && count_sprites(&c, now, SPRITE_CHICK_STAND, SPRITE_CHICK_SLEEP) == 2);
  in.subagents = 0;
  run(&c, &in, &now, 33);
  CHECK(c.chick_count == 0 && c.puff_count == 2);  // each leaves a spark

  in.state = AGENT_PERMISSION;  // stands still, calling with a !
  run(&c, &in, &now, 1500);
  x = c.at.x;
  run(&c, &in, &now, 1000);
  CHECK(c.at.x == x && count_sprites(&c, now, SPRITE_BANG, SPRITE_BANG) == 1);
  CHECK(crew_wait(&c, now) > 0 && crew_wait(&c, now) <= 400);

  in.state = AGENT_DONE;  // cheers with three sparks, then stands still
  run(&c, &in, &now, 33);
  CHECK(c.puff_count == 3);
  run(&c, &in, &now, 3000);
  CHECK(crew_wait(&c, now) == 0);

  in.state = AGENT_IDLE;  // sleeps, and needs no frames
  run(&c, &in, &now, 33);
  CHECK(count_sprites(&c, now, SPRITE_HERON_SLEEP, SPRITE_HERON_SLEEP) == 1);
  CHECK(count_sprites(&c, now, SPRITE_SNORE, SPRITE_SNORE) == 1 && crew_wait(&c, now) == 0);

  fill_rows(t, 0, 11);  // no room at all: it fades out
  run(&c, &in, &now, 500);
  CHECK(!c.placed && c.opacity == 0);
  feed(t, "\x1b[2J");  // room again: it flies back in
  run(&c, &in, &now, 300);
  CHECK(c.placed && c.flying);

  in.present = false;  // the agent left: it fades and the crew is done
  run(&c, &in, &now, 500);
  CHECK(crew_done(&c));
  term_free(t);
}
