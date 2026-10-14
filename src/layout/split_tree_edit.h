// Helpers shared by layout/split_tree*.c. Nothing else includes this.
#pragma once

#include "layout/split_tree.h"

// A split of children with equal sizes; a single child is returned as is.
SplitNode *tree_split(Axis axis, SplitNode **children, int count);

// The leaf holding item. When parent is given it gets the leaf's parent
// (unchanged for the root) and index its place in it.
SplitNode *tree_find(const SplitNode *node, int item, SplitNode **parent, int *index);
