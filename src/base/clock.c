// Wall-clock milliseconds.
#include "base/clock.h"

#include <windows.h>

// 100 ns ticks from 1601 to 1970.
#define EPOCH_TICKS 116444736000000000ULL

uint64_t clock_from_filetime(uint32_t low, uint32_t high) {
  uint64_t ticks = ((uint64_t)high << 32) | low;
  return ticks > EPOCH_TICKS ? (ticks - EPOCH_TICKS) / 10000 : 0;
}

uint64_t clock_now_ms(void) {
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  return clock_from_filetime(ft.dwLowDateTime, ft.dwHighDateTime);
}
