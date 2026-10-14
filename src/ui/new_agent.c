// Starting a new agent.
#include "ui/new_agent.h"

#include <stdlib.h>

#include "app/paths.h"
#include "base/log.h"
#include "base/utf.h"
#include "claude/claude_commands.h"

const char *new_agent_path(App *a) {
  Pane *p = panes_focused(&a->panes);
  return p ? pane_path(p) : NULL;
}

void new_agent_folder_name(App *a, char *out, size_t cap) {
  const char *path = new_agent_path(a);
  out[0] = 0;
  if (path) folder_name(path, out, cap);
}

void new_agent_start(App *a, const char *cwd, bool worktree) {
  const char *command = claude_start_command(worktree);
  log_info("new-agent", "open \"%s\" in %s", command, cwd ? cwd : "home");
  wchar_t *wide = cwd ? utf8_to_wide(cwd, -1) : NULL;
  app_open_pane_in(a, a->shell, wide, command);
  free(wide);
}
