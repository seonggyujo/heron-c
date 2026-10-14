// Wall-clock time as milliseconds since 1970 (UTC), the unit agent files,
// session files and timestamps use.
#pragma once

#include <stdint.h>

uint64_t clock_now_ms(void);
// A Windows FILETIME (100 ns ticks since 1601) given as its two halves.
uint64_t clock_from_filetime(uint32_t low, uint32_t high);
