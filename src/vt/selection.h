// Text selected with the mouse. Ends are kept as row ids (term.h) so the
// selection stays on its text while history scrolls. Both ends are
// inclusive cells.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "vt/term.h"

typedef struct {
  uint64_t row;
  int col;
} TextPos;

typedef struct {
  bool active;  // false until the mouse moves off the first cell
  TextPos anchor, head;
} Selection;

void selection_begin(Selection *s, TextPos at);
void selection_extend(Selection *s, TextPos to);
void selection_clear(Selection *s);

// Selects the word under at (runs of characters that are not spaces or
// brackets, quotes and separators).
void selection_word(Selection *s, const Term *t, TextPos at);

bool selection_contains(const Selection *s, uint64_t row, int col);

// The selected text as UTF-8: lines joined with CRLF, soft-wrapped lines
// joined without a break, trailing spaces dropped. NULL when nothing is
// selected; the caller frees it.
char *selection_text(const Selection *s, const Term *t);
