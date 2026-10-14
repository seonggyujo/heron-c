// How far each agent's folder is from its last commit, for the sidebar
// rows (Heron's wire-changes.ts): git (git/git.h) is asked on a background
// thread and the answers are kept. A folder is asked again when its
// agent's state changes, when the window gets focus, and every 15 s while
// its agent works; an answer that differs redraws.
#pragma once

#include "ui/app.h"

// After the board changed: asks about the folders whose agent changed state.
void changes_watch(App *a);
// Every second: asks again for working agents.
void changes_tick(App *a);
// The window got focus.
void changes_focus(App *a);
// NULL until git answered, and outside a repository.
const GitChanges *changes_of(App *a, const char *path);
