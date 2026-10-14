// A ring of history lines that drops the oldest when full.
#include "vt/scrollback.h"

#include <stdlib.h>

#include "base/mem.h"
#include "vt/line.h"

void scrollback_init(Scrollback *sb, int cap) {
  sb->lines = cap > 0 ? xcalloc((size_t)cap, sizeof(Line)) : NULL;
  sb->cap = cap > 0 ? cap : 0;
  sb->start = sb->count = 0;
  sb->pushed = 0;
}

void scrollback_free(Scrollback *sb) {
  scrollback_clear(sb);
  free(sb->lines);
  sb->lines = NULL;
  sb->cap = 0;
}

void scrollback_push(Scrollback *sb, Line line) {
  sb->pushed++;
  if (sb->cap == 0) {
    line_free(&line);
    return;
  }
  if (sb->count == sb->cap) {
    line_free(&sb->lines[sb->start]);
    sb->lines[sb->start] = line;
    sb->start = (sb->start + 1) % sb->cap;
    return;
  }
  sb->lines[(sb->start + sb->count) % sb->cap] = line;
  sb->count++;
}

const Line *scrollback_line(const Scrollback *sb, int i) {
  return &sb->lines[(sb->start + i) % sb->cap];
}

void scrollback_clear(Scrollback *sb) {
  for (int i = 0; i < sb->count; i++) line_free(&sb->lines[(sb->start + i) % sb->cap]);
  sb->start = sb->count = 0;
}
