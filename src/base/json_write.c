// Writing JSON, compact or indented by two spaces, with the escapes
// serde_json and JSON.stringify use: \" \\ \b \f \n \r \t, other control
// characters as \u00xx, everything else as it is (UTF-8).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/json.h"

static void write_string(Buf *out, const char *s, size_t len) {
  buf_append(out, "\"", 1);
  for (size_t i = 0; i < len; i++) {
    unsigned char c = (unsigned char)s[i];
    const char *esc = c == '"' ? "\\\"" : c == '\\' ? "\\\\" : c == '\b' ? "\\b" : c == '\f' ? "\\f"
                    : c == '\n' ? "\\n" : c == '\r' ? "\\r" : c == '\t' ? "\\t" : NULL;
    if (esc) buf_append_str(out, esc);
    else if (c < 0x20) buf_printf(out, "\\u%04x", c);
    else buf_append(out, &s[i], 1);
  }
  buf_append(out, "\"", 1);
}

// The shortest text that reads back as the same double; whole numbers
// without a fraction.
static void write_number(Buf *out, const Json *v) {
  if (v->text) {
    buf_append_str(out, v->text);
    return;
  }
  double x = v->number;
  if (!isfinite(x)) {
    buf_append_str(out, "null");
    return;
  }
  if (x == floor(x) && fabs(x) < 9007199254740992.0) {
    buf_printf(out, "%.0f", x);
    return;
  }
  char text[32];
  snprintf(text, sizeof text, "%.15g", x);
  if (strtod(text, NULL) != x) snprintf(text, sizeof text, "%.17g", x);
  buf_append_str(out, text);
}

static void newline(Buf *out, bool pretty, int depth) {
  if (!pretty) return;
  buf_append(out, "\n", 1);
  for (int i = 0; i < depth; i++) buf_append(out, "  ", 2);
}

static void write_value(const Json *v, Buf *out, bool pretty, int depth) {
  switch (v->type) {
    case JSON_NULL: buf_append_str(out, "null"); return;
    case JSON_BOOL: buf_append_str(out, v->boolean ? "true" : "false"); return;
    case JSON_NUMBER: write_number(out, v); return;
    case JSON_STRING: write_string(out, v->text, v->len); return;
    default: break;
  }
  bool object = v->type == JSON_OBJECT;
  buf_append(out, object ? "{" : "[", 1);
  for (int i = 0; i < v->count; i++) {
    if (i) buf_append(out, ",", 1);
    newline(out, pretty, depth + 1);
    if (object) {
      write_string(out, v->keys[i], strlen(v->keys[i]));
      buf_append_str(out, pretty ? ": " : ":");
    }
    write_value(v->items[i], out, pretty, depth + 1);
  }
  if (v->count) newline(out, pretty, depth);
  buf_append(out, object ? "}" : "]", 1);
}

void json_write(const Json *v, Buf *out, bool pretty) {
  if (v) write_value(v, out, pretty, 0);
  else buf_append_str(out, "null");
}
