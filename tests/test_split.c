// Tests for the split tree, its pixel boxes and drop zones.
#include "check.h"
#include "layout/drop_zone.h"
#include "layout/split_rects.h"

static const char *shape(const SplitNode *root) {
  static char text[256];
  tree_describe(root, text, sizeof text);
  return text;
}

void test_split_tree(void) {
  int items[5] = {1, 2, 3, 4, 5};
  SplitNode *t = tree_auto(items, 3);
  CHECK_STR(shape(t), "column(row(1 2) 3)");
  t = tree_insert(t, 9, 3, SIDE_RIGHT);
  CHECK_STR(shape(t), "column(row(1 2) row(3 9))");
  t = tree_insert(t, 8, 1, SIDE_LEFT);  // same axis as the parent: a sibling
  CHECK_STR(shape(t), "column(row(8 1 2) row(3 9))");
  t = tree_remove(t, 3);  // row(9) collapses into its parent
  CHECK_STR(shape(t), "column(row(8 1 2) 9)");
  t = tree_move(t, 2, 9, SIDE_CENTER);
  CHECK_STR(shape(t), "column(row(8 1 9) 2)");
  t = tree_move(t, 8, 2, SIDE_BOTTOM);
  CHECK_STR(shape(t), "column(row(1 9) 2 8)");
  int out[8];
  CHECK(tree_items(t, out, 8) == 4 && out[0] == 1 && out[3] == 8);
  t = tree_remove(t, 1);
  t = tree_remove(t, 9);
  t = tree_remove(t, 2);
  CHECK_STR(shape(t), "8");
  t = tree_remove(t, 8);
  CHECK(t == NULL);
  tree_free(tree_auto(items, 5));
}

void test_split_rects(void) {
  int items[3] = {1, 2, 3};
  SplitNode *t = tree_auto(items, 3);
  Rects r = {0};
  rects_compute(&r, t, (Box){10, 10, 190, 90}, 10);
  Box b;
  CHECK(rects_box_of(&r, 1, &b) && b.x == 10 && b.w == 90 && b.h == 40);
  CHECK(rects_box_of(&r, 3, &b) && b.y == 60 && b.w == 190);
  CHECK(r.divider_count == 2);  // under row 1, and between panes 1 and 2
  t->sizes[0] = 0.75;           // what dragging the horizontal divider does
  t->sizes[1] = 0.25;
  rects_compute(&r, t, (Box){10, 10, 190, 90}, 10);
  CHECK(rects_box_of(&r, 3, &b) && b.y == 80 && b.h == 20);
  rects_free(&r);
  tree_free(t);
}

void test_drop_zone(void) {
  Box b = {0, 0, 100, 80};
  CHECK(drop_zone_at(b, 50, 40) == SIDE_CENTER);
  CHECK(drop_zone_at(b, 5, 40) == SIDE_LEFT);
  CHECK(drop_zone_at(b, 95, 70) == SIDE_RIGHT);
  CHECK(drop_zone_at(b, 50, 75) == SIDE_BOTTOM);
  Box p = drop_preview(b, SIDE_TOP);
  CHECK(p.h == 40 && p.y == 0);
}
