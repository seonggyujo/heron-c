// Helpers for terminal tests: feed a C string, read a row back as UTF-8.
#pragma once

#include "vt/term.h"

void feed(Term *t, const char *s);

// Row y of what is shown, trailing empty cells dropped. The buffer is
// reused by the next call.
const char *row_text(const Term *t, int y);

// The replies buffer as a C string, then cleared.
const char *take_replies(Term *t);
