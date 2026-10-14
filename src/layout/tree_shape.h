// A split tree with pane positions in place of panes (Heron's
// layout/tree-shape.ts), as JSON, to save the user's own arrangement and
// build it again for reopened panes:
//   {"kind":"leaf","item":0}
//   {"kind":"split","axis":"row","children":[...],"sizes":[0.5,0.5]}
// A saved tree is used only when it is whole: every pane exactly once, and
// sizes that are positive fractions adding up to 1.
#pragma once

#include "base/json.h"
#include "layout/split_tree.h"

// root with each item replaced by its position in items; NULL when an
// item is not there.
Json *tree_shape_of(const SplitNode *root, const int *items, int count);
// A saved shape as a tree of items, or NULL when it does not fit them.
SplitNode *tree_from_shape(const Json *shape, const int *items, int count);
