// Environment block with some variables replaced or added.
#include "base/env_block.h"

#include <stdbool.h>
#include <string.h>
#include <windows.h>

#include "base/mem.h"

static bool is_set_here(const wchar_t *entry, const EnvVar *vars, int count) {
  for (int i = 0; i < count; i++) {
    size_t n = wcslen(vars[i].name);
    if (_wcsnicmp(entry, vars[i].name, n) == 0 && entry[n] == L'=') return true;
  }
  return false;
}

wchar_t *env_block_with(const EnvVar *vars, int count) {
  wchar_t *env = GetEnvironmentStringsW();
  size_t total = 1;
  for (const wchar_t *p = env; *p; p += wcslen(p) + 1) total += wcslen(p) + 1;
  for (int i = 0; i < count; i++) total += wcslen(vars[i].name) + wcslen(vars[i].value) + 2;
  wchar_t *out = xmalloc(total * sizeof(wchar_t)), *w = out;
  for (const wchar_t *p = env; *p; p += wcslen(p) + 1) {
    size_t len = wcslen(p);
    if (is_set_here(p, vars, count)) continue;
    memcpy(w, p, (len + 1) * sizeof(wchar_t));
    w += len + 1;
  }
  for (int i = 0; i < count; i++) {
    size_t name_len = wcslen(vars[i].name), value_len = wcslen(vars[i].value);
    memcpy(w, vars[i].name, name_len * sizeof(wchar_t));
    w += name_len;
    *w++ = L'=';
    memcpy(w, vars[i].value, (value_len + 1) * sizeof(wchar_t));
    w += value_len + 1;
  }
  *w = 0;  // the block ends with an empty string
  FreeEnvironmentStringsW(env);
  return out;
}
