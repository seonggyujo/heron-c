// Claude Code's config folder: CLAUDE_CONFIG_DIR, or %USERPROFILE%\.claude.
// The hooks find settings.json in it, the recent sessions their files.
#pragma once

#include <stdbool.h>
#include <stddef.h>

// The folder as UTF-8; false when neither variable is set.
bool claude_config_dir(char *out, size_t cap);
