// The VT escape sequence state machine. ESC always starts a new sequence, and
// inside a string (OSC, DCS) it also ends the string, so "ESC \" (ST) needs no
// special case: the trailing '\' arrives as an ESC sequence nobody handles.
#include "vt/parser.h"

#include <string.h>

#include "vt/parser_collect.h"

enum {
  ST_GROUND,
  ST_ESC,
  ST_ESC_INTER,
  ST_CSI,
  ST_CSI_INTER,
  ST_CSI_IGNORE,
  ST_OSC,
  ST_STRING_IGNORE,  // DCS, SOS, PM, APC: skipped until ST
};

void vt_parser_init(VtParser *p, VtHandler h) {
  memset(p, 0, sizeof *p);
  p->h = h;
}

static void flush_utf8(VtParser *p) {
  if (utf8_decoder_reset(&p->utf8)) p->h.print(p->h.user, 0xFFFD);
}

static void dispatch_osc(VtParser *p) {
  p->osc[p->osc_len] = 0;
  if (!p->osc_overflow) p->h.osc(p->h.user, p->osc, p->osc_len);
}

static void ground(VtParser *p, uint8_t b) {
  if (b < 0x20) {
    flush_utf8(p);
    p->h.execute(p->h.user, b);
    return;
  }
  if (b == 0x7F) return;
  uint32_t out[2];
  int n = utf8_decoder_feed(&p->utf8, b, out);
  for (int i = 0; i < n; i++) p->h.print(p->h.user, out[i]);
}

static int string_state(uint8_t b) {
  switch (b) {
    case '[': return ST_CSI;
    case ']': return ST_OSC;
    case 'P':
    case 'X':
    case '^':
    case '_': return ST_STRING_IGNORE;
    default: return ST_GROUND;
  }
}

static void escape(VtParser *p, uint8_t b) {
  if (b < 0x20) {
    p->h.execute(p->h.user, b);
    return;
  }
  if (b <= 0x2F) {
    collect_inter(p, b);
    p->state = ST_ESC_INTER;
    return;
  }
  int next = p->state == ST_ESC ? string_state(b) : ST_GROUND;
  if (next == ST_OSC) collect_osc_begin(p);
  if (next == ST_GROUND && b < 0x7F) p->h.esc(p->h.user, p->inter, b);
  p->state = next;
}

static void csi(VtParser *p, uint8_t b) {
  if (b < 0x20) {
    p->h.execute(p->h.user, b);
  } else if (b >= '0' && b <= '9' && p->state == ST_CSI) {
    collect_digit(p, b);
  } else if ((b == ';' || b == ':') && p->state == ST_CSI) {
    collect_separator(p, b);
  } else if (b >= '<' && b <= '?' && p->state == ST_CSI && p->params.n == 0 && !p->prefix) {
    p->prefix = (char)b;
  } else if (b <= 0x2F) {
    collect_inter(p, b);
    p->state = ST_CSI_INTER;
  } else if (b <= 0x3F) {
    p->state = ST_CSI_IGNORE;
  } else {
    if (b < 0x7F) p->h.csi(p->h.user, p->prefix, &p->params, p->inter, b);
    p->state = ST_GROUND;
  }
}

static void osc(VtParser *p, uint8_t b) {
  if (b == 0x07) {
    dispatch_osc(p);
    p->state = ST_GROUND;
  } else if (b >= 0x20) {
    collect_osc(p, b);
  }
}

void vt_parser_feed(VtParser *p, const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t b = data[i];
    if (b == 0x1B) {
      if (p->state == ST_OSC) dispatch_osc(p);
      if (p->state == ST_GROUND) flush_utf8(p);
      collect_begin(p);
      p->state = ST_ESC;
      continue;
    }
    if (b == 0x18 || b == 0x1A) {  // CAN, SUB: cancel the sequence
      p->state = ST_GROUND;
      continue;
    }
    switch (p->state) {
      case ST_GROUND: ground(p, b); break;
      case ST_ESC:
      case ST_ESC_INTER: escape(p, b); break;
      case ST_CSI:
      case ST_CSI_INTER: csi(p, b); break;
      case ST_CSI_IGNORE:
        if (b < 0x20) p->h.execute(p->h.user, b);
        else if (b >= 0x40 && b <= 0x7E) p->state = ST_GROUND;
        break;
      case ST_OSC: osc(p, b); break;
      default: break;  // ST_STRING_IGNORE
    }
  }
}
