// Building and reading split trees.
#include "layout/split_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/mem.h"
#include "layout/split_tree_edit.h"

SplitNode *tree_leaf(int item) {
  SplitNode *n = xcalloc(1, sizeof *n);
  n->leaf = true;
  n->item = item;
  return n;
}

SplitNode *tree_split(Axis axis, SplitNode **children, int count) {
  if (count == 1) return children[0];
  SplitNode *n = xcalloc(1, sizeof *n);
  n->axis = axis;
  n->count = count;
  n->children = xmalloc(sizeof(SplitNode *) * (size_t)count);
  n->sizes = xmalloc(sizeof(double) * (size_t)count);
  for (int i = 0; i < count; i++) {
    n->children[i] = children[i];
    n->sizes[i] = 1.0 / count;
  }
  return n;
}

void tree_free(SplitNode *node) {
  if (!node) return;
  for (int i = 0; i < node->count; i++) tree_free(node->children[i]);
  free(node->children);
  free(node->sizes);
  free(node);
}

SplitNode *tree_auto(const int *items, int count) {
  if (count <= 0) return NULL;
  int cols = 1;
  while (cols * cols < count) cols++;
  int rows = (count + cols - 1) / cols, at = 0;
  SplitNode **row_nodes = xmalloc(sizeof(SplitNode *) * (size_t)rows);
  SplitNode **cells = xmalloc(sizeof(SplitNode *) * (size_t)cols);
  for (int r = 0; r < rows; r++) {
    int in_row = r < rows - 1 ? cols : count - cols * (rows - 1);
    for (int c = 0; c < in_row; c++) cells[c] = tree_leaf(items[at++]);
    row_nodes[r] = tree_split(AXIS_ROW, cells, in_row);
  }
  SplitNode *root = tree_split(AXIS_COLUMN, row_nodes, rows);
  free(cells);
  free(row_nodes);
  return root;
}

int tree_items(const SplitNode *root, int *out, int cap) {
  if (!root) return 0;
  if (root->leaf) {
    if (cap > 0) out[0] = root->item;
    return 1;
  }
  int n = 0;
  for (int i = 0; i < root->count; i++) {
    n += tree_items(root->children[i], out + (n < cap ? n : cap), cap > n ? cap - n : 0);
  }
  return n;
}

bool tree_contains(const SplitNode *root, int item) { return tree_find(root, item, NULL, NULL) != NULL; }

SplitNode *tree_find(const SplitNode *node, int item, SplitNode **parent, int *index) {
  if (!node) return NULL;
  if (node->leaf) return node->item == item ? (SplitNode *)node : NULL;
  for (int i = 0; i < node->count; i++) {
    SplitNode *hit = tree_find(node->children[i], item, parent, index);
    if (hit) {
      if (parent && hit == node->children[i]) {
        *parent = (SplitNode *)node;
        *index = i;
      }
      return hit;
    }
  }
  return NULL;
}

static void describe(const SplitNode *n, char *out, size_t cap, size_t *len) {
  if (*len >= cap) return;
  if (n->leaf) {
    *len += (size_t)snprintf(out + *len, cap - *len, "%d", n->item);
    return;
  }
  *len += (size_t)snprintf(out + *len, cap - *len, "%s(", n->axis == AXIS_ROW ? "row" : "column");
  for (int i = 0; i < n->count && *len < cap; i++) {
    if (i) *len += (size_t)snprintf(out + *len, cap - *len, " ");
    describe(n->children[i], out, cap, len);
  }
  if (*len < cap) *len += (size_t)snprintf(out + *len, cap - *len, ")");
}

void tree_describe(const SplitNode *root, char *out, size_t cap) {
  size_t len = 0;
  out[0] = 0;
  if (!root) {
    snprintf(out, cap, "empty");
    return;
  }
  describe(root, out, cap, &len);
}
