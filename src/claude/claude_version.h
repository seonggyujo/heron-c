// Claude Code version numbers, as `claude --version` prints them
// ("2.1.296 (Claude Code)") and as each transcript entry records them
// ("2.1.296"). Pure.
#pragma once

#include <stdbool.h>

typedef struct {
  unsigned major, minor, patch;
} ClaudeVersion;

// The leading "major.minor.patch"; false for anything else.
bool claude_version_parse(const char *text, ClaudeVersion *out);
// Negative, zero or positive as a is older than, the same as or newer than b.
int claude_version_compare(ClaudeVersion a, ClaudeVersion b);
