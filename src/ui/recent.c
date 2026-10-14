// Loading, filtering, opening and deleting recent sessions.
#include "ui/recent.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/log.h"
#include "base/mem.h"
#include "base/utf.h"
#include "claude/claude_commands.h"
#include "ui/background.h"

typedef struct {
  RecentSession sessions[RECENT_SESSIONS_MAX];
  int count;
} Loaded;

static void read_list(void *data) {
  Loaded *l = data;
  l->count = sessions_recent(l->sessions, RECENT_SESSIONS_MAX);
}

static bool listed(char ids[][64], int count, const char *id) {
  for (int i = 0; i < count; i++) {
    if (strcmp(ids[i], id) == 0) return true;
  }
  return false;
}

// The sessions to list, newest first, and their folders.
static void regroup(App *a) {
  AgentsState *s = &a->agents;
  s->shown_count = 0;
  for (int i = 0; i < s->session_count; i++) {
    if (!listed(s->running, s->running_count, s->sessions[i].id)) s->shown[s->shown_count++] = s->sessions[i];
  }
  session_groups_free(&s->groups);
  s->groups = session_groups(s->shown, s->shown_count);
}

static void loaded(App *a, void *data) {
  Loaded *l = data;
  AgentsState *s = &a->agents;
  memcpy(s->sessions, l->sessions, sizeof l->sessions);
  s->session_count = l->count;
  s->sessions_busy = false;
  free(l);
  regroup(a);
  log_debug("sessions", "%d recent sessions", s->session_count);
  app_redraw(a);
  if (s->sessions_again) {
    s->sessions_again = false;
    recent_load(a);
  }
}

void recent_load(App *a) {
  AgentsState *s = &a->agents;
  if (s->sessions_busy) {
    s->sessions_again = true;
    return;
  }
  s->sessions_busy = true;
  background_run(a, read_list, loaded, xcalloc(1, sizeof(Loaded)));
}

void recent_running(App *a) {
  AgentsState *s = &a->agents;
  char now[16][64] = {{0}};
  int n = 0;
  for (int i = 0; i < s->board.count && n < 16; i++) {
    const Agent *agent = board_get(&s->board, s->board.entries[i].pane);
    if (agent && agent->session_id[0]) snprintf(now[n++], 64, "%s", agent->session_id);
  }
  bool left = false, changed = n != s->running_count;
  for (int i = 0; i < s->running_count; i++) left |= !listed(now, n, s->running[i]);
  for (int i = 0; i < n; i++) changed |= !listed(s->running, s->running_count, now[i]);
  memcpy(s->running, now, sizeof now);
  s->running_count = n;
  if (left) {
    recent_load(a);
  } else if (changed) {
    regroup(a);
    app_redraw(a);
  }
}

void recent_open(App *a, const RecentSession *s) {
  char command[96];
  if (!claude_resume_command(s->id, command, sizeof command)) return;
  log_info("sessions", "open \"%s\" in %s", command, s->cwd);
  wchar_t *cwd = utf8_to_wide(s->cwd, -1);
  app_open_pane_in(a, a->shell, cwd, command);
  free(cwd);
}

typedef struct {
  char id[40];
  SessionDelete result;
  RecentDeleted done;
} Deletion;

static void delete_job(void *data) {
  Deletion *d = data;
  d->result = sessions_delete(d->id);
}

static void deleted(App *a, void *data) {
  Deletion *d = data;
  AgentsState *s = &a->agents;
  if (d->result == SESSION_DELETED) {
    for (int i = 0; i < s->session_count; i++) {
      if (strcmp(s->sessions[i].id, d->id) != 0) continue;
      memmove(&s->sessions[i], &s->sessions[i + 1], sizeof *s->sessions * (size_t)(s->session_count - i - 1));
      s->session_count--;
      break;
    }
    regroup(a);
  } else {
    log_warn("sessions", "deleting %s failed: %d", d->id, (int)d->result);
  }
  d->done(a, d->result);
  free(d);
  app_redraw(a);
}

void recent_delete(App *a, const char *id, RecentDeleted done) {
  Deletion *d = xcalloc(1, sizeof *d);
  snprintf(d->id, sizeof d->id, "%s", id);
  d->done = done;
  background_run(a, delete_job, deleted, d);
}
