// Streaming UTF-8 decoder.
#include "vt/utf8_decoder.h"

static bool is_valid(uint32_t cp, uint32_t min) {
  return cp >= min && cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF);
}

int utf8_decoder_feed(Utf8Decoder *d, uint8_t b, uint32_t out[2]) {
  int n = 0;
  if (d->need) {
    if ((b & 0xC0) == 0x80) {
      d->cp = (d->cp << 6) | (b & 0x3F);
      if (--d->need) return 0;
      out[0] = is_valid(d->cp, d->min) ? d->cp : 0xFFFD;
      return 1;
    }
    // The sequence was cut short; the byte starts over below.
    d->need = 0;
    out[n++] = 0xFFFD;
  }
  if (b < 0x80) {
    out[n++] = b;
  } else if ((b & 0xE0) == 0xC0) {
    d->cp = b & 0x1F;
    d->need = 1;
    d->min = 0x80;
  } else if ((b & 0xF0) == 0xE0) {
    d->cp = b & 0x0F;
    d->need = 2;
    d->min = 0x800;
  } else if ((b & 0xF8) == 0xF0) {
    d->cp = b & 0x07;
    d->need = 3;
    d->min = 0x10000;
  } else {
    out[n++] = 0xFFFD;
  }
  return n;
}

bool utf8_decoder_reset(Utf8Decoder *d) {
  bool pending = d->need != 0;
  d->need = 0;
  return pending;
}
