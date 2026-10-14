// Shared by buddy/crew*.c only.
#pragma once

#include "buddy/crew.h"

#define CHEER_MS 2400u  // how long a finished turn is cheered
#define CALL_MS 400u    // the call mark bobs at this beat
#define PUFF_MS 450u

// A random number in [lo, hi).
uint32_t crew_random(Crew *c, uint32_t lo, uint32_t hi);
// Where the heron is at now: on its spot, or along its flight.
Point crew_heron_at(const Crew *c, uint64_t now);
bool crew_cheering(const Crew *c, uint64_t now);
void crew_puff(Crew *c, Point at, uint64_t start);

// crew_moves.c
// Refreshes the heron's run, or flies it to the nearest room, or lets it
// go when there is none.
void crew_keep_spot(Crew *c, const CrewInput *in, uint64_t now);
// Walking, pecking and wandering while the agent works.
void crew_busy(Crew *c, const CrewInput *in, float dt, uint64_t now);

// crew_chicks.c: as many chicks as want (at most CREW_CHICKS_MAX).
void chicks_update(Crew *c, int want, float dt, uint64_t now);
