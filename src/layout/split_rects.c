// Split tree to pixel boxes.
#include "layout/split_rects.h"

#include <math.h>
#include <stdlib.h>

#include "base/mem.h"

static void push_box(Rects *r, int item, Box b) {
  r->boxes = xrealloc(r->boxes, sizeof(ItemBox) * (size_t)(r->box_count + 1));
  r->boxes[r->box_count++] = (ItemBox){item, b};
}

static void push_divider(Rects *r, Divider d) {
  r->dividers = xrealloc(r->dividers, sizeof(Divider) * (size_t)(r->divider_count + 1));
  r->dividers[r->divider_count++] = d;
}

// Part of box from..to along the split's axis.
static Box cut(Box b, bool row, int from, int to) {
  if (row) {
    b.x = from;
    b.w = to - from;
  } else {
    b.y = from;
    b.h = to - from;
  }
  return b;
}

static void place(Rects *r, SplitNode *node, Box box, int gap) {
  if (node->leaf) {
    push_box(r, node->item, box);
    return;
  }
  bool row = node->axis == AXIS_ROW;
  int start = row ? box.x : box.y, n = node->count;
  int span = (row ? box.w : box.h) - gap * (n - 1);
  if (span < 0) span = 0;
  // Edge positions along the axis, rounded once so neighbours agree.
  double acc = 0;
  for (int i = 0; i < n; i++) {
    int from = (int)lround(start + span * acc + gap * i);
    acc += node->sizes[i];
    int to = (int)lround(start + span * acc + gap * i);
    place(r, node->children[i], cut(box, row, from, to), gap);
    if (i < n - 1) {
      int next = (int)lround(start + span * acc + gap * (i + 1));
      push_divider(r, (Divider){node, i, node->axis, cut(box, row, to, next), span});
    }
  }
}

void rects_compute(Rects *r, SplitNode *root, Box area, int gap) {
  r->box_count = r->divider_count = 0;
  if (root) place(r, root, area, gap);
}

void rects_free(Rects *r) {
  free(r->boxes);
  free(r->dividers);
  r->boxes = NULL;
  r->dividers = NULL;
  r->box_count = r->divider_count = 0;
}

bool rects_box_of(const Rects *r, int item, Box *out) {
  for (int i = 0; i < r->box_count; i++) {
    if (r->boxes[i].item == item) {
      *out = r->boxes[i].box;
      return true;
    }
  }
  return false;
}
