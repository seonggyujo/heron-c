// Allocation that never returns NULL. Running out of memory ends the program
// with a log line, so callers never handle half-built state.
#pragma once

#include <stddef.h>

void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *p, size_t size);
