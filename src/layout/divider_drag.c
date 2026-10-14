// Divider drag and double click.
#include "layout/divider_drag.h"

bool divider_drag_begin(DividerDrag *drag, const Divider *d, int pos, unsigned generation) {
  if (d->span <= 0) return false;
  double first = d->split->sizes[d->index], pair = first + d->split->sizes[d->index + 1];
  double min = (double)DIVIDER_MIN_PX / d->span;
  *drag = (DividerDrag){d->split, d->index, d->axis, pos, d->span, first, pair,
                        min < pair / 2 ? min : pair / 2, generation};
  return true;
}

bool divider_drag_move(DividerDrag *drag, int pos) {
  double next = drag->first + (double)(pos - drag->origin) / drag->span;
  if (next < drag->min) next = drag->min;
  if (next > drag->pair - drag->min) next = drag->pair - drag->min;
  double *sizes = drag->split->sizes;
  if (next == sizes[drag->index]) return false;
  sizes[drag->index] = next;
  sizes[drag->index + 1] = drag->pair - next;
  return true;
}

bool divider_equalize(const Divider *d) {
  double *sizes = d->split->sizes;
  double half = (sizes[d->index] + sizes[d->index + 1]) / 2;
  if (sizes[d->index] == half) return false;
  sizes[d->index] = sizes[d->index + 1] = half;
  return true;
}
