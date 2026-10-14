// Growable byte buffer, used for replies to the shell, pty output queues and
// pasted text. A zeroed Buf is a valid empty buffer.
#pragma once

#include <stddef.h>

typedef struct {
  char *data;
  size_t len, cap;
} Buf;

void buf_append(Buf *b, const void *data, size_t n);
void buf_append_str(Buf *b, const char *s);

#if defined(__clang__)
__attribute__((format(printf, 2, 3)))
#elif defined(__GNUC__)
__attribute__((format(gnu_printf, 2, 3)))
#endif
void buf_printf(Buf *b, const char *fmt, ...);

void buf_clear(Buf *b);
void buf_free(Buf *b);
