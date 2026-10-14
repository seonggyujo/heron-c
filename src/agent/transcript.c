// Reading the new lines of a transcript.
#include "agent/transcript.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/clock.h"
#include "base/file.h"

// More new bytes than this between two reads: start over at the end.
#define MAX_NEW_BYTES (16ULL * 1024 * 1024)

void transcript_follow(Transcript *t, const char *path) {
  memset(t, 0, sizeof *t);
  snprintf(t->path, sizeof t->path, "%s", path);
  file_info(path, &t->offset, NULL);
  t->since = clock_now_ms();
}

static void read_line(Transcript *t, char *line, size_t len, AnswerVisit fn, void *ctx) {
  Answer answer;
  if (!answer_of(line, len, &answer)) return;
  if (answer.id[0] && strcmp(answer.id, t->last_id) == 0) return;  // another line of the same answer
  snprintf(t->last_id, sizeof t->last_id, "%s", answer.id);
  if (!answer.has_at || answer.at >= t->since) fn(ctx, &answer);
}

void transcript_read_new(Transcript *t, AnswerVisit fn, void *ctx) {
  uint64_t size;
  if (!file_info(t->path, &size, NULL) || size == t->offset) return;
  if (size < t->offset || size - t->offset > MAX_NEW_BYTES) {
    t->offset = size;
    return;
  }
  size_t len;
  char *bytes = file_read_range(t->path, t->offset, size, &len);
  if (!bytes) return;
  char *end = NULL;
  for (size_t i = len; i > 0; i--) {
    if (bytes[i - 1] == '\n') {
      end = bytes + i;
      break;
    }
  }
  if (end) {
    t->offset += (uint64_t)(end - bytes);
    for (char *line = bytes; line < end;) {
      char *nl = memchr(line, '\n', (size_t)(end - line));
      size_t n = (size_t)(nl - line);
      if (n && line[n - 1] == '\r') n--;
      line[n] = 0;
      read_line(t, line, n, fn, ctx);
      line = nl + 1;
    }
  }
  free(bytes);
}
