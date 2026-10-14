// Turning the hooks on and off in the background.
#include "ui/hooks_state.h"

#include <stdio.h>
#include <stdlib.h>

#include "app/prefs.h"
#include "app/strings.h"
#include "base/log.h"
#include "base/mem.h"
#include "hooks/hooks.h"
#include "ui/background.h"

typedef struct {
  bool want;
  HookPresence presence;
  HookResult result;
} HookJob;

// The sentence for a failure (Heron's i18n/hooks-error.ts).
static void error_text(HookResult r, char *out, size_t cap) {
  const Strings *s = strings();
  switch (r.error) {
    case HOOKS_NO_CLAUDE: snprintf(out, cap, "%s", s->hooks_no_claude); break;
    case HOOKS_CLAUDE_OLD: format_hooks_old_claude(out, cap, r.version); break;
    case HOOKS_BAD_SETTINGS: snprintf(out, cap, "%s", s->hooks_bad_settings); break;
    default: snprintf(out, cap, "%s", s->hooks_failed); break;
  }
}

static void change(void *data) {
  HookJob *j = data;
  j->result = j->want ? hooks_install() : hooks_remove();
}

static void changed(App *a, void *data) {
  HookJob *j = data;
  AgentsState *s = &a->agents;
  s->hooks_busy = false;
  if (j->result.error == HOOKS_OK) {
    prefs_set_bool(PREF_AGENT_HOOKS, j->want);
    s->hooks_on = j->want;
    s->hooks_error[0] = 0;
    log_info("hooks", "claude hooks %s", j->want ? "on" : "off");
  } else {
    error_text(j->result, s->hooks_error, sizeof s->hooks_error);
    log_warn("hooks", "claude hooks %s failed: %d", j->want ? "on" : "off", (int)j->result.error);
  }
  free(j);
  app_redraw(a);
}

void hooks_set(App *a, bool on) {
  if (a->agents.hooks_busy) return;
  a->agents.hooks_busy = true;
  a->agents.hooks_error[0] = 0;
  HookJob *j = xcalloc(1, sizeof *j);
  j->want = on;
  background_run(a, change, changed, j);
  app_redraw(a);
}

static void check(void *data) {
  HookJob *j = data;
  j->result = hooks_presence(&j->presence);
}

static void checked(App *a, void *data) {
  HookJob *j = data;
  if (j->result.error != HOOKS_OK) {
    log_warn("hooks", "claude hooks check failed: %d", (int)j->result.error);
  } else {
    static const char *const NAMES[] = {"absent", "current", "stale"};
    log_debug("hooks", "claude hooks %s", NAMES[j->presence]);
    a->agents.hooks_on = j->presence != HOOKS_ABSENT;
    // Hooks of this heron-c.exe were turned on here, also when a reset forgot that.
    if (j->presence == HOOKS_CURRENT) prefs_set_bool(PREF_AGENT_HOOKS, true);
    else if (prefs_get_bool(PREF_AGENT_HOOKS, false)) hooks_set(a, true);
    app_redraw(a);
  }
  free(j);
}

void hooks_sync(App *a) { background_run(a, check, checked, xcalloc(1, sizeof(HookJob))); }
