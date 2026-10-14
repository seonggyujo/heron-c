// Growable byte buffer. Capacity doubles so appends stay amortized O(1).
#include "base/buf.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/mem.h"

static void reserve(Buf *b, size_t extra) {
  if (b->len + extra <= b->cap) return;
  size_t cap = b->cap ? b->cap : 256;
  while (cap < b->len + extra) cap *= 2;
  b->data = xrealloc(b->data, cap);
  b->cap = cap;
}

void buf_append(Buf *b, const void *data, size_t n) {
  if (n == 0) return;
  reserve(b, n);
  memcpy(b->data + b->len, data, n);
  b->len += n;
}

void buf_append_str(Buf *b, const char *s) { buf_append(b, s, strlen(s)); }

void buf_printf(Buf *b, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(NULL, 0, fmt, ap);
  va_end(ap);
  if (n <= 0) return;
  // One more byte for the NUL vsnprintf writes; len does not count it.
  reserve(b, (size_t)n + 1);
  va_start(ap, fmt);
  vsnprintf(b->data + b->len, (size_t)n + 1, fmt, ap);
  va_end(ap);
  b->len += (size_t)n;
}

void buf_clear(Buf *b) { b->len = 0; }

void buf_free(Buf *b) {
  free(b->data);
  b->data = NULL;
  b->len = b->cap = 0;
}
