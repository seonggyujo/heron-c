// Notes over several lines: lines break only at spaces, so a Korean word
// stays whole (CSS word-break: keep-all), and each line is drawn as one
// line of ui_text.
#include <string.h>

#include "render/ui_text.h"

#define LINE_MAX_BYTES 1024

static const char *word_end(const char *p) {
  while (*p && *p != ' ') p++;
  return p;
}

static void copy_span(char *out, const char *from, const char *to) {
  size_t n = (size_t)(to - from);
  if (n >= LINE_MAX_BYTES) n = LINE_MAX_BYTES - 1;
  memcpy(out, from, n);
  out[n] = 0;
}

// The end of the line that starts at p: as many words as fit in w, and at
// least one.
static const char *line_end(UiFont *f, const char *p, float w) {
  char line[LINE_MAX_BYTES];
  const char *end = word_end(p);
  for (;;) {
    const char *next = end;
    while (*next == ' ') next++;
    if (!*next) return end;
    const char *longer = word_end(next);
    copy_span(line, p, longer);
    if (ui_text_width(f, line) > w) return end;
    end = longer;
  }
}

int ui_text_wrap(Canvas *c, UiFont *f, const char *utf8, float x, float y, float w, float line_h,
                 uint32_t argb) {
  if (!f || w <= 0) return 0;
  int lines = 0;
  const char *p = utf8;
  for (;;) {
    while (*p == ' ') p++;
    if (!*p) return lines;
    const char *end = line_end(f, p, w);
    if (c) {
      char line[LINE_MAX_BYTES];
      copy_span(line, p, end);
      ui_text_draw(c, f, line, x, y + lines * line_h, w, line_h, argb, ALIGN_LEFT);
    }
    lines++;
    p = end;
  }
}
