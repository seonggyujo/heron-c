// The open panes in the order they were opened, the focused one, where they
// sit (layout/arrangement.h), and closed panes on their way out. A closed
// pane first leaves (it is off the list but keeps its place while its exit
// animation plays), is then released (its place goes to the others), and
// is freed once its shell has exited too. pane_list.c keeps the lists and
// focus; pane_arrange.c opens, closes and places panes.
#pragma once

#include <stdbool.h>

#include "layout/arrangement.h"
#include "pane/pane.h"

typedef struct {
  Pane **items;
  int count, cap;
} PaneArray;

typedef struct {
  PaneArray shown;    // on screen, in the order they were opened
  PaneArray leaving;  // closed, still holding their place
  PaneArray closing;  // released, waiting for their shell to exit
  Arrangement arrangement;
  int focused_id;
  int next_id;
} PaneList;

// Where the panes go: the workspace below the title bar.
typedef struct {
  Box workspace;
  int gap;
  const PaneMetrics *m;
  const MonoFont *font;
} PaneLayout;

// pane_arrange.c
// Opens a pane and focuses it. In manual mode it takes half of the pane
// that had focus.
Pane *panes_open(PaneList *l, const PaneStart *start, const PaneHost *host, const PaneLayout *layout);
// The id the next opened pane gets, for what its shell is told.
int panes_next_id(const PaneList *l);
// Takes a pane off the list and ends its shell. It keeps its place until
// panes_release. Returns the pane that takes its place in the list (it
// should get focus, as in Heron), or NULL when none is left.
Pane *panes_close(PaneList *l, Pane *p);
// Gives a closed pane's place to the others.
void panes_release(PaneList *l, Pane *p, const PaneLayout *layout);
// A dropped pane: split target on zone's side, or swap the two.
void panes_move(PaneList *l, Pane *p, Pane *target, Side zone, const PaneLayout *layout);
// Back to the automatic grid.
void panes_tidy(PaneList *l, const PaneLayout *layout);
// A divider changed the sizes: the arrangement is the user's now.
void panes_resized(PaneList *l, const PaneLayout *layout);
void panes_layout(PaneList *l, const PaneLayout *layout);
// The user's own arrangement to save, NULL for the automatic grid.
Json *panes_shape(const PaneList *l);
// Puts back a saved arrangement of the shown panes; false when it does not
// fit them.
bool panes_restore(PaneList *l, const Json *shape, const PaneLayout *layout);

// pane_list.c
// A shell exited. A released pane is freed. Returns the shown pane it
// belonged to, NULL otherwise.
Pane *panes_exited(PaneList *l, int id, DWORD code);
Pane *panes_find(const PaneList *l, int id);  // shown, leaving or closing
Pane *panes_focused(const PaneList *l);
void panes_focus(PaneList *l, Pane *p);
void panes_focus_step(PaneList *l, int step);
Pane *panes_at(const PaneList *l, int x, int y);  // shown panes only
int panes_running(const PaneList *l);
void panes_kill_all(PaneList *l);

// Shared by pane_list.c and pane_arrange.c.
void pane_array_push(PaneArray *a, Pane *p);
// Returns where p was, -1 when it was not there.
int pane_array_remove(PaneArray *a, const Pane *p);
