// Everything watched for one pane, one round at a time (Heron's
// agent/watch.rs and subagents.rs): the agent files (passed on as they
// are), the running subagents, and the model check from the transcript the
// hooks name and the model they report (or, while they do not know it,
// the model the heron-limits status line shows). Writers replace files in
// one step, so a read never sees half a file; one that cannot be read yet
// is tried again next round.
#pragma once

#include "agent/agent_files.h"
#include "agent/model_check.h"
#include "agent/transcript.h"

// One update for the agent board. kind: "state", "status", "check" or
// "subagents". data is owned by whoever gets the update.
typedef struct {
  const char *kind;
  Json *data;
} AgentUpdate;

#define PANE_WATCH_UPDATES 4

typedef struct {
  bool seen[AGENT_FILE_KINDS];
  uint64_t modified[AGENT_FILE_KINDS];
  bool following;
  Transcript transcript;
  ModelCheck check;
  Json *sent;       // the verdict last sent
  Json *subagents;  // the list last sent
} PaneWatch;

// The updates of this round, in order; returns how many. New answers are
// judged before a new selection is taken, so an answer that came before a
// switch is judged against the model it was asked of.
int pane_watch_poll(PaneWatch *w, const char *dir, int pane, AgentUpdate out[PANE_WATCH_UPDATES]);
void pane_watch_free(PaneWatch *w);
