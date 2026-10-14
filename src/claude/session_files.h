// Finding Claude Code's session files and the sessions running now. Only
// claude/sessions*.c include this.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SESSION_PATH_MAX 1024

typedef struct {
  char id[40];
  char path[SESSION_PATH_MAX];
  uint64_t modified;
} SessionFile;

// Every <config>\projects\<project>\<name>.jsonl (subagent files sit
// deeper). The caller frees the list.
int session_files(const char *config, SessionFile **out);

// Ids of the sessions some Claude Code process runs now: each running
// process keeps <config>\sessions\<pid>.json; a file whose process is gone
// (a crash) does not count.
typedef struct {
  int count;
  char (*ids)[40];
} RunningSessions;

RunningSessions sessions_running(const char *config);
bool sessions_is_running(const RunningSessions *r, const char *id);
void sessions_running_free(RunningSessions *r);
