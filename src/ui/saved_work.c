// Reopening and saving the work.
#include "ui/saved_work.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/prefs.h"
#include "app/workspace.h"
#include "base/log.h"
#include "base/mem.h"
#include "base/utf.h"
#include "claude/claude_commands.h"
#include "ui/agents.h"
#include "ui/background.h"

typedef struct {
  SavedWorkspace saved;
  bool *resumable;
} Reopen;

// Off the UI thread: which saved sessions Claude Code can resume.
static void check_sessions(void *data) {
  Reopen *r = data;
  for (int i = 0; i < r->saved.count; i++) {
    const char *id = r->saved.panes[i].session;
    r->resumable[i] = id[0] && sessions_resumable(id);
  }
}

// What to type at the pane's first prompt, NULL for nothing.
static const char *command_of(const SavedPane *p, bool resumable, char *buf, size_t cap) {
  if (!p->session[0] || !claude_resume_command(p->session, buf, cap)) return NULL;
  return resumable ? buf : claude_start_command(false);
}

static void open_rest(App *a) {
  if (a->start_dir || a->panes.shown.count == 0) app_open_pane(a);
  a->work_open = true;
}

static void reopen(App *a, void *data) {
  Reopen *r = data;
  int resumed = 0;
  for (int i = 0; i < r->saved.count; i++) {
    const SavedPane *p = &r->saved.panes[i];
    ShellKind shell;
    if (!shell_from_id(p->shell, &shell) || !shell_installed(shell)) shell = a->shell;
    char buf[96];
    const char *command = command_of(p, r->resumable[i], buf, sizeof buf);
    resumed += command == buf;
    wchar_t *cwd = p->cwd[0] ? utf8_to_wide(p->cwd, -1) : NULL;
    app_open_pane_in(a, shell, cwd, command);
    free(cwd);
  }
  PaneLayout l = app_pane_layout(a);
  bool arranged = r->saved.layout && panes_restore(&a->panes, r->saved.layout, &l);
  log_info("workspace", "reopened %d panes, %d sessions resumed%s", r->saved.count, resumed,
           arranged ? ", own layout" : "");
  workspace_free(&r->saved);
  free(r->resumable);
  free(r);
  open_rest(a);
  app_redraw(a);
}

void saved_work_open(App *a) {
  char *text = prefs_get_bool(PREF_RESTORE_WORKSPACE, true) ? workspace_read() : NULL;
  Reopen *r = xcalloc(1, sizeof *r);
  bool saved = text && workspace_parse(text, &r->saved) && r->saved.count > 0;
  free(text);
  if (!saved) {
    if (r->saved.panes) workspace_free(&r->saved);
    free(r);
    open_rest(a);
    return;
  }
  r->resumable = xcalloc((size_t)r->saved.count, sizeof *r->resumable);
  background_run(a, check_sessions, reopen, r);
}

static char *snapshot(App *a) {
  int n = a->panes.shown.count;
  SavedPane *panes = xcalloc((size_t)(n ? n : 1), sizeof *panes);
  for (int i = 0; i < n; i++) {
    Pane *p = a->panes.shown.items[i];
    const Agent *agent = agents_of(a, p);
    snprintf(panes[i].shell, sizeof panes[i].shell, "%s", shell_id(p->shell));
    snprintf(panes[i].cwd, sizeof panes[i].cwd, "%s", pane_path(p) ? pane_path(p) : "");
    snprintf(panes[i].session, sizeof panes[i].session, "%s", agent ? agent->session_id : "");
  }
  Json *layout = panes_shape(&a->panes);
  char *text = workspace_text(panes, n, layout);
  json_free(layout);
  free(panes);
  return text;
}

void saved_work_tick(App *a) {
  if (!a->work_open) return;
  char *text = snapshot(a);
  if (a->saved_work && strcmp(text, a->saved_work) == 0) {
    free(text);
    return;
  }
  workspace_write(text);
  free(a->saved_work);
  a->saved_work = text;
}
