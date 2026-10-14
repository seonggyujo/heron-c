// The VT escape sequence state machine (after the DEC parser described at
// vt100.net). It turns a byte stream into actions: print a character, run a
// control code, or dispatch an ESC, CSI or OSC sequence. It knows nothing
// about what those actions mean; the terminal handles them.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "vt/utf8_decoder.h"

#define VT_MAX_PARAMS 32
#define VT_MAX_INTER 4
#define VT_OSC_MAX 4096

typedef struct {
  int n;                      // how many values
  int v[VT_MAX_PARAMS];       // -1 where the value was left out
  bool colon[VT_MAX_PARAMS];  // true when the value followed ':' (a sub-parameter)
} VtParams;

typedef struct {
  void *user;
  void (*print)(void *user, uint32_t cp);
  void (*execute)(void *user, uint8_t control);
  // inter holds the intermediate bytes, NUL-terminated.
  void (*esc)(void *user, const char *inter, uint8_t final);
  // prefix is the private marker ('?', '>', '<', '=') or 0.
  void (*csi)(void *user, char prefix, const VtParams *p, const char *inter, uint8_t final);
  // data is NUL-terminated.
  void (*osc)(void *user, const char *data, size_t len);
} VtHandler;

typedef struct {
  int state;
  VtHandler h;
  Utf8Decoder utf8;
  VtParams params;
  char prefix;
  char inter[VT_MAX_INTER + 1];
  int ninter;
  char osc[VT_OSC_MAX + 1];
  size_t osc_len;
  bool osc_overflow;
} VtParser;

void vt_parser_init(VtParser *p, VtHandler h);
void vt_parser_feed(VtParser *p, const uint8_t *data, size_t len);

// Parameter i, or def when it was left out.
static inline int vt_param(const VtParams *p, int i, int def) {
  return (i < p->n && p->v[i] >= 0) ? p->v[i] : def;
}

// Parameter i as a count: left out or 0 both mean 1.
static inline int vt_count(const VtParams *p, int i) {
  int v = vt_param(p, i, 1);
  return v < 1 ? 1 : v;
}
