// Changing a split tree: insert, remove, move, swap.
#include <stdlib.h>
#include <string.h>

#include "base/mem.h"
#include "layout/split_tree_edit.h"

// Opens a slot at index in a split's children and sizes.
static void open_slot(SplitNode *s, int index) {
  s->children = xrealloc(s->children, sizeof(SplitNode *) * (size_t)(s->count + 1));
  s->sizes = xrealloc(s->sizes, sizeof(double) * (size_t)(s->count + 1));
  memmove(&s->children[index + 1], &s->children[index], sizeof(SplitNode *) * (size_t)(s->count - index));
  memmove(&s->sizes[index + 1], &s->sizes[index], sizeof(double) * (size_t)(s->count - index));
  s->count++;
}

static void close_slot(SplitNode *s, int index) {
  memmove(&s->children[index], &s->children[index + 1], sizeof(SplitNode *) * (size_t)(s->count - index - 1));
  memmove(&s->sizes[index], &s->sizes[index + 1], sizeof(double) * (size_t)(s->count - index - 1));
  s->count--;
}

// The parent of node (by pointer) and its index there; NULL for the root.
static SplitNode *parent_of(SplitNode *root, const SplitNode *node, int *index) {
  if (!root || root->leaf) return NULL;
  for (int i = 0; i < root->count; i++) {
    if (root->children[i] == node) {
      *index = i;
      return root;
    }
    SplitNode *p = parent_of(root->children[i], node, index);
    if (p) return p;
  }
  return NULL;
}

SplitNode *tree_insert(SplitNode *root, int item, int target, Side side) {
  SplitNode *parent = NULL;
  int index = 0;
  SplitNode *node = tree_find(root, target, &parent, &index);
  if (!node || side == SIDE_CENTER) return root;
  Axis axis = side == SIDE_LEFT || side == SIDE_RIGHT ? AXIS_ROW : AXIS_COLUMN;
  bool before = side == SIDE_LEFT || side == SIDE_TOP;
  if (parent && parent->axis == axis) {  // a sibling, no extra nesting
    double half = parent->sizes[index] / 2;
    open_slot(parent, before ? index : index + 1);
    parent->children[before ? index : index + 1] = tree_leaf(item);
    parent->sizes[index] = parent->sizes[index + 1] = half;
    return root;
  }
  SplitNode *pair[2] = {before ? tree_leaf(item) : node, before ? node : tree_leaf(item)};
  SplitNode *split = tree_split(axis, pair, 2);
  if (!parent) return split;
  parent->children[index] = split;
  return root;
}

// Puts next where old was; a split of the parent's own axis is merged in.
static SplitNode *replace(SplitNode *root, SplitNode *old, SplitNode *next) {
  int index = 0;
  SplitNode *parent = parent_of(root, old, &index);
  if (!parent) return next;
  if (!next->leaf && next->axis == parent->axis) {
    double share = parent->sizes[index];
    close_slot(parent, index);
    for (int i = 0; i < next->count; i++) {
      open_slot(parent, index + i);
      parent->children[index + i] = next->children[i];
      parent->sizes[index + i] = next->sizes[i] * share;
    }
    next->count = 0;
    tree_free(next);
  } else {
    parent->children[index] = next;
  }
  return root;
}

SplitNode *tree_remove(SplitNode *root, int item) {
  SplitNode *parent = NULL;
  int index = 0;
  SplitNode *node = tree_find(root, item, &parent, &index);
  if (!node) return root;
  tree_free(node);
  if (!parent) return NULL;
  double freed = parent->sizes[index], rest = 1 - freed;
  close_slot(parent, index);
  for (int i = 0; i < parent->count; i++) parent->sizes[i] = rest > 0 ? parent->sizes[i] / rest : 1.0 / parent->count;
  if (parent->count > 1) return root;
  SplitNode *only = parent->children[0];  // a split of one is replaced by that child
  parent->count = 0;
  SplitNode *result = replace(root, parent, only);
  tree_free(parent);
  return result;
}

SplitNode *tree_move(SplitNode *root, int item, int target, Side side) {
  SplitNode *a = tree_find(root, item, NULL, NULL), *b = tree_find(root, target, NULL, NULL);
  if (item == target || !a || !b) return root;
  if (side == SIDE_CENTER) {
    a->item = target;
    b->item = item;
    return root;
  }
  SplitNode *rest = tree_remove(root, item);
  return rest ? tree_insert(rest, item, target, side) : root;
}
