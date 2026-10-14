// Preparing pasted text for the shell: line breaks become CR, as pressing
// Enter would send, and in bracketed paste mode the text is wrapped in the
// start and end markers, with ESC removed so the text cannot end the paste
// early.
#pragma once

#include <stdbool.h>
#include <stddef.h>

// Returns malloc'd bytes and their count in out_len.
char *paste_prepare(const char *utf8, bool bracketed, size_t *out_len);
