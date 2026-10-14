// What git says about a folder, for the agent sidebar (Heron's git/): is it
// in a repository (a new agent can start in a worktree there), and how far
// is the repository's working tree from HEAD. Runs the git on PATH without
// a console window and without git's optional locks, so an agent's own git
// commands never meet a lock of ours. Blocks: call it off the UI thread.
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "git/numstat.h"

// The top folder of the working tree dir is in: the nearest folder, dir or
// above, with a .git folder (a repository) or file (a worktree).
bool git_root(const char *dir, char *out, size_t cap);

// Tracked changes since HEAD, staged or not, plus untracked files that are
// not ignored. False outside a repository or when git fails.
bool git_changes(const char *dir, GitChanges *out);
