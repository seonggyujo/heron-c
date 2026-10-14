// A model id the way the sidebar shows it (Heron's agent/model-label.ts),
// pure: "claude-opus-5-5" → "Opus 5.5", "claude-sonnet-5-5[1m]" →
// "Sonnet 5.5 1M", "claude-haiku-5-5-20260301" → "Haiku 5.5". An alias
// ("opus") or an id of another shape keeps its words, without the
// "claude-" in front.
#pragma once

#include <stddef.h>

void model_label(const char *id, char *out, size_t cap);
