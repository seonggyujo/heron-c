// Follows one Claude Code transcript while it grows (Heron's
// agent/transcript.rs): each read returns the answers in the complete
// lines added since the last read. It starts at the end of the file, and
// skips answers written before it started (a forked or continued session
// begins with copies of older ones), because the model selected for those
// is unknown.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "agent/answers.h"

#define TRANSCRIPT_PATH_MAX 1024

typedef struct {
  char path[TRANSCRIPT_PATH_MAX];
  uint64_t offset;
  uint64_t since;  // when following began, ms since 1970
  char last_id[ANSWER_TEXT_MAX];
} Transcript;

void transcript_follow(Transcript *t, const char *path);
// Calls fn with each new answer, one per response, oldest first. A line
// still being written is left for the next read.
typedef void (*AnswerVisit)(void *ctx, const Answer *answer);
void transcript_read_new(Transcript *t, AnswerVisit fn, void *ctx);
