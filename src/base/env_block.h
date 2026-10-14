// A copy of this process's environment with some variables set, in the
// UTF-16 block form CreateProcessW takes.
#pragma once

#include <wchar.h>

typedef struct {
  const wchar_t *name, *value;
} EnvVar;

// Returns a malloc'd block. A variable of the same name (compared without
// case, as Windows does) is replaced.
wchar_t *env_block_with(const EnvVar *vars, int count);
