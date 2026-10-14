// Saving and rebuilding a split tree's shape.
#include "layout/tree_shape.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "base/mem.h"
#include "layout/split_tree_edit.h"

Json *tree_shape_of(const SplitNode *root, const int *items, int count) {
  if (!root) return NULL;
  Json *v = json_new(JSON_OBJECT);
  if (root->leaf) {
    int index = -1;
    for (int i = 0; i < count; i++) {
      if (items[i] == root->item) index = i;
    }
    json_set(v, "kind", json_new_string("leaf"));
    json_set(v, "item", json_new_number(index));
    if (index >= 0) return v;
    json_free(v);
    return NULL;
  }
  Json *children = json_new(JSON_ARRAY), *sizes = json_new(JSON_ARRAY);
  json_set(v, "kind", json_new_string("split"));
  json_set(v, "axis", json_new_string(root->axis == AXIS_ROW ? "row" : "column"));
  json_set(v, "children", children);
  json_set(v, "sizes", sizes);
  for (int i = 0; i < root->count; i++) {
    Json *child = tree_shape_of(root->children[i], items, count);
    if (!child) {
      json_free(v);
      return NULL;
    }
    json_push(children, child);
    json_push(sizes, json_new_number(root->sizes[i]));
  }
  return v;
}

static bool valid_sizes(const Json *sizes, int count) {
  if (!json_is(sizes, JSON_ARRAY) || sizes->count != count) return false;
  double sum = 0, s;
  for (int i = 0; i < count; i++) {
    if (!json_number(sizes->items[i], &s) || !(s > 0)) return false;
    sum += s;
  }
  return fabs(sum - 1) < 0.01;
}

static SplitNode *build(const Json *v, const int *items, int count, bool *used) {
  const char *kind = json_string(json_get(v, "kind"));
  if (kind && strcmp(kind, "leaf") == 0) {
    double i;
    if (!json_number(json_get(v, "item"), &i) || i != floor(i) || i < 0 || i >= count || used[(int)i]) return NULL;
    used[(int)i] = true;
    return tree_leaf(items[(int)i]);
  }
  const char *axis = json_string(json_get(v, "axis"));
  const Json *children = json_get(v, "children"), *sizes = json_get(v, "sizes");
  if (!kind || strcmp(kind, "split") != 0 || !axis || (strcmp(axis, "row") != 0 && strcmp(axis, "column") != 0) ||
      !json_is(children, JSON_ARRAY) || children->count < 2 || !valid_sizes(sizes, children->count)) {
    return NULL;
  }
  SplitNode **built = xcalloc((size_t)children->count, sizeof *built);
  bool ok = true;
  for (int i = 0; i < children->count && ok; i++) ok = (built[i] = build(children->items[i], items, count, used)) != NULL;
  SplitNode *node = NULL;
  if (ok) {
    node = tree_split(strcmp(axis, "row") == 0 ? AXIS_ROW : AXIS_COLUMN, built, children->count);
    for (int i = 0; i < children->count; i++) json_number(sizes->items[i], &node->sizes[i]);
  } else {
    for (int i = 0; i < children->count; i++) tree_free(built[i]);
  }
  free(built);
  return node;
}

SplitNode *tree_from_shape(const Json *shape, const int *items, int count) {
  bool *used = xcalloc((size_t)count + 1, sizeof *used);
  SplitNode *tree = build(shape, items, count, used);
  for (int i = 0; tree && i < count; i++) {
    if (!used[i]) {
      tree_free(tree);
      tree = NULL;
    }
  }
  free(used);
  return tree;
}
