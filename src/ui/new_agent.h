// "New agent" (Heron's wire-new-agent.ts): a pane with the default shell
// that types `claude` at its first prompt, in the selected pane's folder,
// or wherever the folder menu (ui/agent_folder_menu.h) says, with
// `--worktree` for a new git worktree of its own.
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "ui/app.h"

// The selected pane's folder, NULL for the home folder.
const char *new_agent_path(App *a);
// Its name for the button, "" for the home folder.
void new_agent_folder_name(App *a, char *out, size_t cap);
// cwd NULL: the home folder.
void new_agent_start(App *a, const char *cwd, bool worktree);
