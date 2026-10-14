// Finding plain http and https URLs in a line of text, with the rules of
// xterm.js's web-links addon: the body stops at spaces, quotes, brackets and
// a few symbols, and trailing punctuation such as a final period is not part
// of the URL. Non-ASCII letters (a Korean path, say) stay in it.
#pragma once

#include <stdbool.h>
#include <stdint.h>

// The URL covering code point at, as [*start, *end). Returns false when
// there is none.
bool url_find(const uint32_t *text, int n, int at, int *start, int *end);
