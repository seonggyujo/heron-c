// Size, DPI, activation and pty events.
#include "ui/app_events.h"

#include <stdlib.h>
#include <string.h>

#include "app/prefs.h"
#include "base/log.h"
#include "pane/pane_input.h"
#include "ui/agents.h"
#include "ui/animation.h"
#include "ui/changes.h"
#include "ui/recent.h"
#include "ui/popover.h"

void app_on_size(App *a, WPARAM kind, int width, int height) {
  popover_close(a);
  // A minimized window has no size; fitting panes to it would shrink every
  // shell to one row.
  if (kind == SIZE_MINIMIZED) return;
  canvas_resize(a->canvas, width, height);
  app_layout(a);
  app_redraw(a);
}

void app_on_dpi(App *a, UINT dpi, const RECT *suggested) {
  app_set_dpi(a, dpi);
  SetWindowPos(a->hwnd, NULL, suggested->left, suggested->top, suggested->right - suggested->left,
               suggested->bottom - suggested->top, SWP_NOZORDER | SWP_NOACTIVATE);
  app_layout(a);
  app_redraw(a);
}

void app_on_activate(App *a, bool active) {
  a->active = active;
  if (!active) popover_close(a);
  Pane *p = panes_focused(&a->panes);
  if (p) pane_focus_changed(p, active);
  if (active) {
    if (p) agents_seen(a, p);
    changes_focus(a);
    recent_load(a);
  }
  app_redraw(a);
}

void app_on_output(App *a, int pane_id) {
  Pane *p = panes_find(&a->panes, pane_id);
  if (!p) return;
  unsigned events = pane_drain(p);
  if (events & TERM_EVENT_TITLE) {
    log_debug("pane", "pane %d title: %s", p->id, term_title(p->term));
    agents_title(a, p);
  }
  if (events & TERM_EVENT_CWD) log_debug("cwd", "pane %d in %s", p->id, term_cwd(p->term));
  if (events & TERM_EVENT_PROMPT) {
    agents_prompt(a, p);
    if (p->start_command) {  // typed once, as if by the user, at the first prompt
      log_info("pane", "pane %d: typing %s", p->id, p->start_command);
      pane_send(p, p->start_command, strlen(p->start_command));
      pane_send(p, "\r", 1);
      free(p->start_command);
      p->start_command = NULL;
    }
  }
  anim_output(a, p);
  app_redraw(a);
}

void app_on_exit(App *a, int pane_id, DWORD code) {
  Pane *p = panes_exited(&a->panes, pane_id, code);
  if (p && code == 0 && prefs_get_bool(PREF_CLOSE_ON_EXIT, true)) app_close_pane(a, p);
  app_redraw(a);
}
