// Streaming UTF-8 decoder: bytes in, code points out, one byte at a time so a
// character split across two reads still decodes. Bad input becomes U+FFFD.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t cp, min;
  int need;  // continuation bytes still expected
} Utf8Decoder;

// Feeds one byte and returns how many code points were written to out:
// 0 while a sequence is unfinished, 2 when a broken sequence yields U+FFFD
// followed by the byte that broke it.
int utf8_decoder_feed(Utf8Decoder *d, uint8_t byte, uint32_t out[2]);

// Drops an unfinished sequence. Returns true if there was one.
bool utf8_decoder_reset(Utf8Decoder *d);
