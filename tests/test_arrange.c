// Tests for the automatic grid, manual arrangement, divider drags and easing.
#include "base/easing.h"
#include "check.h"
#include "layout/arrangement.h"
#include "layout/divider_drag.h"

static const char *shape(const Arrangement *a) {
  static char text[256];
  tree_describe(a->root, text, sizeof text);
  return text;
}

// Heron's grid: cols = ceil(sqrt(n)), the last row stretched.
void test_auto_grid(void) {
  Arrangement a = {0};
  int live[5] = {1, 2, 3, 4, 5};
  Box b;
  arrange_tidy(&a, live, 1);
  arrange_place(&a, (Box){0, 0, 100, 50}, 10);
  CHECK(rects_box_of(&a.rects, 1, &b) && b.x == 10 && b.y == 10 && b.w == 80 && b.h == 30);
  arrange_tidy(&a, live, 3);  // two on top, one wide below
  arrange_place(&a, (Box){0, 0, 210, 110}, 10);
  CHECK(rects_box_of(&a.rects, 1, &b) && b.w == 90);
  CHECK(rects_box_of(&a.rects, 2, &b) && b.x == 110 && b.w == 90);
  CHECK(rects_box_of(&a.rects, 3, &b) && b.x == 10 && b.w == 190 && b.y == 60 && b.h == 40);
  arrange_tidy(&a, live, 5);  // three on top, two below
  arrange_place(&a, (Box){0, 0, 400, 300}, 10);
  Box first, third;
  rects_box_of(&a.rects, 1, &first);
  rects_box_of(&a.rects, 3, &third);
  CHECK(third.y == first.y && rects_box_of(&a.rects, 5, &b) && b.y > first.y && b.x + b.w == 390);
  arrange_free(&a);
}

void test_arrangement(void) {
  Arrangement a = {0};
  Box ws = {0, 40, 1000, 600};
  int live[4] = {1, 2, 3, 4};
  arrange_add(&a, 1, 0, live, 1);
  arrange_add(&a, 2, 1, live, 2);
  arrange_add(&a, 3, 2, live, 3);
  CHECK_STR(shape(&a), "column(row(1 2) 3)");
  unsigned before = a.generation;
  arrange_move(&a, 3, 1, SIDE_LEFT);  // a drop switches to manual
  CHECK(a.manual && a.generation != before);
  CHECK_STR(shape(&a), "row(3 1 2)");
  arrange_place(&a, ws, 10);
  Box b;
  CHECK(rects_box_of(&a.rects, 3, &b) && b.x == 10 && b.y == 50 && b.h == 580);
  arrange_add(&a, 4, 2, live, 4);  // pane 2 is tall: split below it
  CHECK_STR(shape(&a), "row(3 1 column(2 4))");
  int after[3] = {1, 2, 4}, left[2] = {1, 4};
  arrange_remove(&a, 3, after, 3);
  arrange_remove(&a, 2, left, 2);
  CHECK(a.manual);
  CHECK_STR(shape(&a), "row(1 4)");
  arrange_remove(&a, 4, left, 1);  // one left: back to the grid
  CHECK(!a.manual);
  CHECK_STR(shape(&a), "1");
  arrange_free(&a);
}

void test_divider_drag(void) {
  Arrangement a = {0};
  int live[2] = {1, 2};
  arrange_tidy(&a, live, 2);
  arrange_place(&a, (Box){0, 0, 420, 300}, 10);  // the two share 390 px
  CHECK(a.rects.divider_count == 1);
  Divider d = a.rects.dividers[0];
  DividerDrag drag;
  CHECK(divider_drag_begin(&drag, &d, d.box.x, a.generation));
  CHECK(divider_drag_move(&drag, d.box.x + 39));
  CHECK(a.root->sizes[0] > 0.599 && a.root->sizes[0] < 0.601);
  divider_drag_move(&drag, d.box.x + 500);  // stops 80 px short of the edge
  CHECK(a.root->sizes[1] > 80.0 / 390 - 1e-9 && a.root->sizes[1] < 80.0 / 390 + 1e-9);
  CHECK(divider_equalize(&d) && a.root->sizes[0] > 0.4999 && a.root->sizes[0] < 0.5001);
  CHECK(!divider_equalize(&d));
  arrange_free(&a);
}

void test_easing(void) {
  CHECK(ease(EASE_OUT, 0) == 0 && ease(EASE_OUT, 1) == 1);
  CHECK(ease(EASE_OUT, 0.5f) > 0.85f);  // most of the way there by half time
  CHECK(ease(EASE_IN, 0.5f) < 0.35f);
  float last = 0;
  for (int i = 1; i <= 20; i++) {
    float v = ease(EASE_OUT, i / 20.0f);
    CHECK(v >= last);
    last = v;
  }
}
