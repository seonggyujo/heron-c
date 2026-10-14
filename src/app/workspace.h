// The work saved to reopen on the next start (Heron's app/workspace.ts and
// workspace-store.ts): the panes in the order they were opened, each with
// its shell, its folder and the Claude Code session that ran in it, and
// the user's own arrangement (layout/tree_shape.h checks that one when it
// is put back). Kept in %APPDATA%\heron-c\workspace.json; a failure only
// means nothing reopens.
#pragma once

#include <stdbool.h>

#include "base/json.h"

typedef struct {
  char shell[16];    // shell id; the caller falls back to the default for an unknown one
  char cwd[1024];    // empty: home
  char session[64];  // empty: none; the caller checks it is a session id
} SavedPane;

typedef struct {
  SavedPane *panes;
  int count;
  Json *layout;  // NULL for the automatic grid
} SavedWorkspace;

// The saved text as panes and layout; false when it is missing or broken.
bool workspace_parse(const char *text, SavedWorkspace *out);
void workspace_free(SavedWorkspace *w);
// {"panes":[{"shell","cwd","session"}],"layout"} as compact JSON text.
char *workspace_text(const SavedPane *panes, int count, const Json *layout);

// The saved text, NULL when there is none.
char *workspace_read(void);
void workspace_write(const char *text);
