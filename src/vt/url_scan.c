// URL scanning. The two character sets are those of the web-links regex
//   (https?):[/]{2}[^\s"'!*(){}|\\^<>`]*[^\s"':,.!?{}|\\^~\[\]`()<>]
#include "vt/url_scan.h"

#include <string.h>

static bool in_set(uint32_t c, const char *set) { return c < 128 && c && strchr(set, (int)c); }

static bool body_char(uint32_t c) { return c > ' ' && !in_set(c, "\"'!*(){}|\\^<>`"); }

static bool end_char(uint32_t c) { return c > ' ' && !in_set(c, "\"':,.!?{}|\\^~[]`()<>"); }

static uint32_t lower(uint32_t c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

// Length of "http://" or "https://" at i, or 0.
static int scheme_at(const uint32_t *t, int n, int i) {
  static const char *const SCHEMES[] = {"https://", "http://"};
  for (int s = 0; s < 2; s++) {
    int len = (int)strlen(SCHEMES[s]), k = 0;
    while (k < len && i + k < n && lower(t[i + k]) == (uint32_t)SCHEMES[s][k]) k++;
    if (k == len) return len;
  }
  return 0;
}

bool url_find(const uint32_t *text, int n, int at, int *start, int *end) {
  for (int i = 0; i <= at && i < n; i++) {
    int scheme = scheme_at(text, n, i);
    if (!scheme) continue;
    int j = i + scheme;
    while (j < n && body_char(text[j])) j++;
    while (j > i + scheme && !end_char(text[j - 1])) j--;
    if (j == i + scheme) continue;  // nothing after "//"
    if (at < j) {
      *start = i;
      *end = j;
      return true;
    }
    i = j - 1;  // the next URL can only start after this one
  }
  return false;
}
