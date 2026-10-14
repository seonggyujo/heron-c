// Tests for transcript answers, the model check and following a
// transcript (Heron's answers.rs, model_check.rs and transcript.rs tests).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "agent/model_check.h"
#include "agent/transcript.h"
#include "check.h"

static bool answer(const char *extra, const char *model, Answer *out) {
  char line[512];
  snprintf(line, sizeof line,
           "{\"type\":\"assistant\",\"isSidechain\":false,\"timestamp\":\"2026-10-10T00:18:30.302Z\",%s"
           "\"message\":{\"id\":\"msg_1\",\"model\":\"%s\"}}",
           extra, model);
  return answer_of(line, strlen(line), out);
}

void test_answers(void) {
  Answer a;
  CHECK(answer("", "claude-opus-5-5", &a) && strcmp(a.id, "msg_1") == 0 && a.at == 1791591510302ULL);
  CHECK(!answer("\"isSidechain\":true,", "claude-opus-5-5", &a));
  CHECK(!answer("", "<synthetic>", &a));
  CHECK(!answer("\"version\":\"2.1.250\",", "claude-opus-5-5", &a));
  CHECK(answer("\"version\":\"2.1.251\",", "claude-opus-5-5", &a));
  const char *user = "{\"type\":\"user\",\"message\":{\"content\":\"assistant\"}}";
  CHECK(!answer_of(user, strlen(user), &a));
  CHECK(same_model("claude-opus-5-5[1m]", "claude-opus-5-5"));
  CHECK(same_model("claude-haiku-4-5-20251001", "Claude-Haiku-4-5"));
  CHECK(!same_model("claude-sonnet-5-5", "claude-opus-5-5"));
  uint64_t ms;
  CHECK(iso_millis("1970-01-01T00:00:00Z", &ms) && ms == 0);
  CHECK(iso_millis("2000-03-01T00:00:01.5Z", &ms) && ms == 951868801500ULL);
  CHECK(!iso_millis("2026-10-10T00:18:30+09:00", &ms) && !iso_millis("yesterday", &ms));
}

static const char *state(const ModelCheck *c) {
  static char text[16];
  Json *v = model_check_verdict(c);
  snprintf(text, sizeof text, "%s", json_string(json_get(v, "state")));
  json_free(v);
  return text;
}

void test_model_check(void) {
  ModelCheck c = {0};
  CHECK_STR(state(&c), "pending");
  model_check_select(&c, "claude-opus-5-5");
  model_check_answered(&c, "claude-opus-5-5[1m]");
  CHECK_STR(state(&c), "ok");
  model_check_answered(&c, "claude-sonnet-5-5");
  CHECK_STR(state(&c), "mismatch");
  model_check_select(&c, "claude-sonnet-5-5");  // a switch after an answer waits for the next one
  model_check_answered(&c, "claude-opus-5-5");
  model_check_select(&c, "claude-haiku-5-5");
  CHECK_STR(state(&c), "pending");
  ModelCheck resumed = {0};  // the hooks do not know; the status line speaks
  model_check_select(&resumed, NULL);
  model_check_select_from_status_line(&resumed, "claude-sonnet-5-5");
  model_check_answered(&resumed, "claude-opus-5-5");
  CHECK_STR(state(&resumed), "mismatch");
  model_check_select(&resumed, NULL);
  model_check_restart(&resumed);
  model_check_answered(&resumed, "claude-opus-5-5");
  CHECK_STR(state(&resumed), "pending");
}

typedef struct {
  int count;
  char models[8][ANSWER_TEXT_MAX];
} Found;

static void found(void *ctx, const Answer *a) {
  Found *f = ctx;
  if (f->count < 8) snprintf(f->models[f->count++], ANSWER_TEXT_MAX, "%s", a->model);
}

static void append(const char *path, const char *text) {
  FILE *f = fopen(path, "ab");
  fputs(text, f);
  fclose(f);
}

static void line(char *out, size_t cap, const char *id, const char *model, const char *ts) {
  snprintf(out, cap, "{\"type\":\"assistant\",\"timestamp\":\"%s\",\"message\":{\"id\":\"%s\",\"model\":\"%s\"}}\n", ts,
           id, model);
}

void test_transcript(void) {
  char path[MAX_PATH], text[512], half[512];
  GetTempPathA(MAX_PATH, path);
  snprintf(path + strlen(path), MAX_PATH - strlen(path), "heron-c-transcript-%lu.jsonl", GetCurrentProcessId());
  remove(path);
  line(text, sizeof text, "old", "claude-sonnet-5-5", "2099-01-01T00:00:00Z");
  append(path, text);
  Transcript t;
  transcript_follow(&t, path);
  Found f = {0};
  transcript_read_new(&t, found, &f);
  CHECK(f.count == 0);  // answers already there are not judged
  line(text, sizeof text, "a", "claude-opus-5-5", "2099-01-01T00:00:01Z");
  append(path, text);
  append(path, text);  // lines of one answer count once
  snprintf(half, sizeof half, "%.10s", text);
  append(path, half);
  transcript_read_new(&t, found, &f);
  CHECK(f.count == 1 && strcmp(f.models[0], "claude-opus-5-5") == 0);
  append(path, text + 10);
  transcript_read_new(&t, found, &f);
  CHECK(f.count == 1);  // the finished line was the same answer
  line(text, sizeof text, "copy", "claude-sonnet-5-5", "2000-01-01T00:00:00Z");  // a forked session's copy
  append(path, text);
  line(text, sizeof text, "b", "claude-haiku-5-5", "2099-01-01T00:00:01Z");
  append(path, text);
  transcript_read_new(&t, found, &f);
  CHECK(f.count == 2 && strcmp(f.models[1], "claude-haiku-5-5") == 0);
  remove(path);
}
