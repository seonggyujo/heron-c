// Opening, closing, moving and placing panes through the arrangement.
#include <stdlib.h>

#include "base/mem.h"
#include "pane/pane_list.h"

// Ids of the shown panes in the order they were opened, with room for
// extra more. Free with free().
static int *shown_ids(const PaneList *l, int extra) {
  int *ids = xmalloc(sizeof(int) * (size_t)(l->shown.count + extra + 1));
  for (int i = 0; i < l->shown.count; i++) ids[i] = l->shown.items[i]->id;
  return ids;
}

// Shown and leaving panes have a place; released ones are out of the tree.
void panes_layout(PaneList *l, const PaneLayout *layout) {
  Arrangement *a = &l->arrangement;
  arrange_place(a, layout->workspace, layout->gap);
  for (int i = 0; i < a->rects.box_count; i++) {
    Pane *p = panes_find(l, a->rects.boxes[i].item);
    if (p) pane_place(p, a->rects.boxes[i].box, layout->m, layout->font);
  }
}

int panes_next_id(const PaneList *l) { return l->next_id + 1; }

Pane *panes_open(PaneList *l, const PaneStart *start, const PaneHost *host, const PaneLayout *layout) {
  int id = ++l->next_id;
  int *ids = shown_ids(l, 1);
  ids[l->shown.count] = id;
  arrange_add(&l->arrangement, id, l->focused_id, ids, l->shown.count + 1);
  free(ids);
  // Start the shell at the size it will have, so it never sees a resize.
  arrange_place(&l->arrangement, layout->workspace, layout->gap);
  Box box = {0};
  rects_box_of(&l->arrangement.rects, id, &box);
  int cols, rows;
  pane_grid_size(box, layout->m, layout->font, &cols, &rows);
  Pane *p = pane_open(id, start, cols, rows, host);
  pane_array_push(&l->shown, p);
  l->focused_id = id;
  panes_layout(l, layout);
  return p;
}

Pane *panes_close(PaneList *l, Pane *p) {
  int i = pane_array_remove(&l->shown, p);
  if (i < 0) return NULL;
  if (l->focused_id == p->id) l->focused_id = 0;
  pane_kill(p);
  pane_array_push(&l->leaving, p);
  int n = l->shown.count;
  return n ? l->shown.items[i < n ? i : n - 1] : NULL;
}

void panes_release(PaneList *l, Pane *p, const PaneLayout *layout) {
  if (pane_array_remove(&l->leaving, p) < 0) return;
  int *ids = shown_ids(l, 0);
  arrange_remove(&l->arrangement, p->id, ids, l->shown.count);
  free(ids);
  if (pane_running(p)) {
    pane_array_push(&l->closing, p);  // freed when its exit arrives
  } else {
    pane_free(p);
  }
  panes_layout(l, layout);
}

void panes_move(PaneList *l, Pane *p, Pane *target, Side zone, const PaneLayout *layout) {
  arrange_move(&l->arrangement, p->id, target->id, zone);
  panes_layout(l, layout);
}

void panes_tidy(PaneList *l, const PaneLayout *layout) {
  int *ids = shown_ids(l, 0);
  arrange_tidy(&l->arrangement, ids, l->shown.count);
  free(ids);
  panes_layout(l, layout);
}

Json *panes_shape(const PaneList *l) {
  int *ids = shown_ids(l, 0);
  Json *shape = arrange_shape(&l->arrangement, ids, l->shown.count);
  free(ids);
  return shape;
}

bool panes_restore(PaneList *l, const Json *shape, const PaneLayout *layout) {
  int *ids = shown_ids(l, 0);
  bool ok = arrange_restore(&l->arrangement, shape, ids, l->shown.count);
  free(ids);
  if (ok) panes_layout(l, layout);
  return ok;
}

void panes_resized(PaneList *l, const PaneLayout *layout) {
  arrange_set_manual(&l->arrangement, true);
  panes_layout(l, layout);
}
