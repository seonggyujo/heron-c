// Allocation that never returns NULL.
#include "base/mem.h"

#include <stdio.h>
#include <stdlib.h>

// Not log_error: the log converts text to UTF-16, which allocates.
static void out_of_memory(size_t size) {
  fprintf(stderr, "heron-c: out of memory (%llu bytes)\n", (unsigned long long)size);
  abort();
}

void *xmalloc(size_t size) {
  void *p = malloc(size ? size : 1);
  if (!p) out_of_memory(size);
  return p;
}

void *xcalloc(size_t count, size_t size) {
  void *p = calloc(count ? count : 1, size ? size : 1);
  if (!p) out_of_memory(count * size);
  return p;
}

void *xrealloc(void *p, size_t size) {
  void *q = realloc(p, size ? size : 1);
  if (!q) out_of_memory(size);
  return q;
}
