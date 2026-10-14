// Whether heron-c's Claude Code hooks are on (Heron's app/agent-hooks.ts),
// shared by the sidebar offer and Settings. The choice is remembered
// (PREF_AGENT_HOOKS, which Reset keeps), so at start hooks that an
// uninstall took away, or that run a moved heron-c.exe, are put back.
// Checking, adding and removing run on a background thread; meanwhile
// hooks_busy is set, and a failure leaves its reason in hooks_error.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

void hooks_sync(App *a);
void hooks_set(App *a, bool on);
