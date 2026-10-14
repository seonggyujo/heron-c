// Answers in a Claude Code transcript, one JSON entry per line (Heron's
// agent/answers.rs): which model wrote each answer of the main
// conversation. Pure: parsing and comparing only.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ANSWER_TEXT_MAX 128

typedef struct {
  // message.id: one answer is written as several lines (thinking, text,
  // tool calls) that share it. Empty when there is none.
  char id[ANSWER_TEXT_MAX];
  char model[ANSWER_TEXT_MAX];
  bool has_at;
  uint64_t at;  // when it was written, ms since 1970
} Answer;

// The answer on one line, false for anything else. Subagent entries
// (isSidechain), <synthetic> placeholders and answers of a Claude Code
// older than 2.1.251 (which cannot report model switches) are skipped.
bool answer_of(const char *line, size_t len, Answer *out);

// Same model, ignoring a suffix such as [1m] and a date such as -20251001.
bool same_model(const char *a, const char *b);

// "2026-10-10T05:09:03.787Z" in ms since 1970: the UTC form Claude Code
// writes. False for anything else.
bool iso_millis(const char *ts, uint64_t *out);
