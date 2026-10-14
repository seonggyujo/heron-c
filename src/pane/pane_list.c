// The pane lists, focus, and finding panes.
#include "pane/pane_list.h"

#include <string.h>

#include "base/mem.h"

void pane_array_push(PaneArray *a, Pane *p) {
  if (a->count == a->cap) {
    a->cap = a->cap ? a->cap * 2 : 8;
    a->items = xrealloc(a->items, sizeof(Pane *) * (size_t)a->cap);
  }
  a->items[a->count++] = p;
}

static int index_of(const PaneArray *a, const Pane *p) {
  for (int i = 0; i < a->count; i++) {
    if (a->items[i] == p) return i;
  }
  return -1;
}

int pane_array_remove(PaneArray *a, const Pane *p) {
  int i = index_of(a, p);
  if (i < 0) return -1;
  memmove(&a->items[i], &a->items[i + 1], sizeof(Pane *) * (size_t)(a->count - i - 1));
  a->count--;
  return i;
}

static Pane *find_in(const PaneArray *a, int id) {
  for (int i = 0; i < a->count; i++) {
    if (a->items[i]->id == id) return a->items[i];
  }
  return NULL;
}

Pane *panes_exited(PaneList *l, int id, DWORD code) {
  Pane *p = find_in(&l->closing, id);
  if (p) {
    pane_array_remove(&l->closing, p);
    pane_free(p);
    return NULL;
  }
  p = find_in(&l->leaving, id);
  if (p) {
    pane_set_exited(p, code);  // freed when released
    return NULL;
  }
  p = find_in(&l->shown, id);
  if (p) pane_set_exited(p, code);
  return p;
}

Pane *panes_find(const PaneList *l, int id) {
  Pane *p = find_in(&l->shown, id);
  if (!p) p = find_in(&l->leaving, id);
  return p ? p : find_in(&l->closing, id);
}

Pane *panes_focused(const PaneList *l) { return find_in(&l->shown, l->focused_id); }

void panes_focus(PaneList *l, Pane *p) { l->focused_id = p ? p->id : 0; }

void panes_focus_step(PaneList *l, int step) {
  int n = l->shown.count;
  if (n == 0) return;
  int i = index_of(&l->shown, panes_focused(l));
  i = ((i < 0 ? 0 : i) + step + n) % n;
  l->focused_id = l->shown.items[i]->id;
}

Pane *panes_at(const PaneList *l, int x, int y) {
  for (int i = 0; i < l->shown.count; i++) {
    if (box_contains(l->shown.items[i]->frame, x, y)) return l->shown.items[i];
  }
  return NULL;
}

int panes_running(const PaneList *l) {
  int n = 0;
  for (int i = 0; i < l->shown.count; i++) n += pane_running(l->shown.items[i]);
  return n;
}

void panes_kill_all(PaneList *l) {
  for (int i = 0; i < l->shown.count; i++) pane_kill(l->shown.items[i]);
}
