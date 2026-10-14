// Automatic and manual pane arrangement.
#include "layout/arrangement.h"

#include "base/log.h"
#include "layout/tree_shape.h"

static void changed(Arrangement *a, const char *what) {
  a->generation++;
  if (!log_enabled(LOG_DEBUG)) return;
  char shape[256];
  tree_describe(a->root, shape, sizeof shape);
  log_debug("layout", "%s: %s", what, shape);
}

void arrange_set_manual(Arrangement *a, bool manual) {
  if (a->manual == manual) return;
  a->manual = manual;
  log_info("layout", "%s", manual ? "manual layout" : "automatic grid");
}

static void rebuild(Arrangement *a, const int *live, int count) {
  tree_free(a->root);
  a->root = tree_auto(live, count);
}

static int last_leaf(const SplitNode *n) {
  while (!n->leaf) n = n->children[n->count - 1];
  return n->item;
}

void arrange_add(Arrangement *a, int item, int near, const int *live, int count) {
  if (a->manual && a->root) {
    int target = tree_contains(a->root, near) ? near : last_leaf(a->root);
    Box box;
    bool wide = !rects_box_of(&a->rects, target, &box) || box.w >= box.h;
    a->root = tree_insert(a->root, item, target, wide ? SIDE_RIGHT : SIDE_BOTTOM);
  } else {
    rebuild(a, live, count);
  }
  changed(a, "add");
}

void arrange_remove(Arrangement *a, int item, const int *live, int count) {
  if (a->manual && a->root && count > 1) {
    a->root = tree_remove(a->root, item);
  } else {
    arrange_set_manual(a, false);
    rebuild(a, live, count);
  }
  changed(a, "remove");
}

void arrange_move(Arrangement *a, int item, int target, Side zone) {
  if (!a->root || item == target || !tree_contains(a->root, item) || !tree_contains(a->root, target)) return;
  a->root = tree_move(a->root, item, target, zone);
  arrange_set_manual(a, true);
  changed(a, zone == SIDE_CENTER ? "swap" : "move");
}

void arrange_tidy(Arrangement *a, const int *live, int count) {
  arrange_set_manual(a, false);
  rebuild(a, live, count);
  changed(a, "tidy");
}

Json *arrange_shape(const Arrangement *a, const int *live, int count) {
  return a->manual ? tree_shape_of(a->root, live, count) : NULL;
}

bool arrange_restore(Arrangement *a, const Json *shape, const int *live, int count) {
  SplitNode *tree = tree_from_shape(shape, live, count);
  if (!tree) return false;
  tree_free(a->root);
  a->root = tree;
  arrange_set_manual(a, true);
  changed(a, "restore");
  return true;
}

void arrange_place(Arrangement *a, Box workspace, int gap) {
  int w = workspace.w - 2 * gap, h = workspace.h - 2 * gap;
  Box area = {workspace.x + gap, workspace.y + gap, w > 0 ? w : 0, h > 0 ? h : 0};
  rects_compute(&a->rects, a->root, area, gap);
}

void arrange_free(Arrangement *a) {
  tree_free(a->root);
  rects_free(&a->rects);
  a->root = NULL;
  a->manual = false;
}
