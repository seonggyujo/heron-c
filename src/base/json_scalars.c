// Reading JSON numbers and strings strictly (RFC 8259): \u escapes turned
// into UTF-8, a lone surrogate an error (as in serde_json).
#include <stdlib.h>
#include <string.h>

#include "base/json_reader.h"
#include "base/mem.h"
#include "base/utf.h"

bool json_reader_take(JsonReader *r, const char *word) {
  size_t n = strlen(word);
  if ((size_t)(r->end - r->at) < n || memcmp(r->at, word, n) != 0) return false;
  r->at += n;
  return true;
}

static bool is_digit(JsonReader *r) { return r->at < r->end && *r->at >= '0' && *r->at <= '9'; }

static void digits(JsonReader *r) {
  while (is_digit(r)) r->at++;
}

Json *json_read_number(JsonReader *r) {
  const char *start = r->at;
  if (r->at < r->end && *r->at == '-') r->at++;
  if (!is_digit(r)) return NULL;
  if (*r->at == '0') r->at++;
  else digits(r);
  if (r->at < r->end && *r->at == '.') {
    r->at++;
    if (!is_digit(r)) return NULL;
    digits(r);
  }
  if (r->at < r->end && (*r->at == 'e' || *r->at == 'E')) {
    r->at++;
    if (r->at < r->end && (*r->at == '+' || *r->at == '-')) r->at++;
    if (!is_digit(r)) return NULL;
    digits(r);
  }
  Json *v = json_new(JSON_NUMBER);
  size_t n = (size_t)(r->at - start);
  v->text = xmalloc(n + 1);
  memcpy(v->text, start, n);
  v->text[n] = 0;
  v->number = strtod(v->text, NULL);
  return v;
}

static int hex4(JsonReader *r) {
  if (r->end - r->at < 4) return -1;
  int value = 0;
  for (int i = 0; i < 4; i++) {
    char c = *r->at++;
    int d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
    if (d < 0) return -1;
    value = value * 16 + d;
  }
  return value;
}

// After a backslash: appends what the escape stands for.
static bool escape(JsonReader *r, Buf *out) {
  if (r->at >= r->end) return false;
  char c = *r->at++;
  const char *from = "\"\\/bfnrt", *to = "\"\\/\b\f\n\r\t";
  const char *hit = c ? strchr(from, c) : NULL;
  if (hit) {
    buf_append(out, &to[hit - from], 1);
    return true;
  }
  if (c != 'u') return false;
  int cp = hex4(r);
  if (cp >= 0xDC00 && cp <= 0xDFFF) return false;
  if (cp >= 0xD800 && cp <= 0xDBFF) {
    if (!json_reader_take(r, "\\u")) return false;
    int low = hex4(r);
    if (low < 0xDC00 || low > 0xDFFF) return false;
    cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
  }
  if (cp < 0) return false;
  char bytes[4];
  buf_append(out, bytes, utf8_encode((uint32_t)cp, bytes));
  return true;
}

bool json_read_string(JsonReader *r, Buf *out) {
  for (;;) {
    if (r->at >= r->end) return false;
    unsigned char c = (unsigned char)*r->at++;
    if (c == '"') break;
    if (c < 0x20) return false;
    if (c == '\\') {
      if (!escape(r, out)) return false;
    } else {
      buf_append(out, (const char *)&c, 1);
    }
  }
  buf_append(out, "", 1);
  return true;
}
